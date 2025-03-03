/*
 * cmd_ac.c
 * vim: expandtab:ts=4:sts=4:sw=4
 *
 * Copyright (C) 2012 - 2019 James Booth <boothj5@gmail.com>
 * Copyright (C) 2019 - 2023 Michael Vetter <jubalh@iodoru.org>
 *
 * This file is part of Profanity.
 *
 * Profanity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Profanity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Profanity.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link the code of portions of this program with the OpenSSL library under
 * certain conditions as described in each individual source file, and
 * distribute linked combinations including the two.
 *
 * You must obey the GNU General Public License in all respects for all of the
 * code used other than OpenSSL. If you modify file(s) with this exception, you
 * may extend this exception to your version of the file(s), but you are not
 * obligated to do so. If you do not wish to do so, delete this exception
 * statement from your version. If you delete this exception statement from all
 * source files in the program, then also delete it here.
 *
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <dirent.h>
#include <ctype.h>

#include "common.h"
#include "config/preferences.h"
#include "config/scripts.h"
#include "command/cmd_ac.h"
#include "command/cmd_funcs.h"
#include "tools/parser.h"
#include "plugins/plugins.h"
#include "ui/ui.h"
#include "ui/win_types.h"
#include "ui/window_list.h"
#include "xmpp/muc.h"
#include "xmpp/xmpp.h"
#include "xmpp/roster_list.h"
#include "ui/buffer.h"

#ifdef HAVE_LIBGPGME
#include "pgp/gpg.h"
#endif

#ifdef HAVE_OMEMO
#include "omemo/omemo.h"
#endif

static char* _sub_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _notify_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _theme_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _autoaway_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _autoconnect_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _account_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _who_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _roster_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _bookmark_autocomplete(ProfWin* window, const char* const input, gboolean previous);
#ifdef HAVE_LIBOTR
static char* _otr_autocomplete(ProfWin* window, const char* const input, gboolean previous);
#endif
#ifdef HAVE_LIBGPGME
static char* _pgp_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _ox_autocomplete(ProfWin* window, const char* const input, gboolean previous);
#endif
#ifdef HAVE_OMEMO
static char* _omemo_autocomplete(ProfWin* window, const char* const input, gboolean previous);
#endif
static char* _connect_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _alias_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _join_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _log_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _form_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _form_field_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _occupants_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _kick_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _ban_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _affiliation_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _role_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _resource_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _wintitle_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _inpblock_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _time_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _receipts_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _help_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _wins_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _tls_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _titlebar_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _script_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _subject_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _console_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _win_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _close_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _plugins_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _sendfile_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _blocked_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _tray_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _presence_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _rooms_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _statusbar_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _clear_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _invite_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _status_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _logging_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _color_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _avatar_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _correction_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _correct_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _software_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _url_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _executable_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _lastactivity_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _intype_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _mood_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _strophe_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _adhoc_cmd_autocomplete(ProfWin* window, const char* const input, gboolean previous);
static char* _vcard_autocomplete(ProfWin* window, const char* const input, gboolean previous);

static char* _script_autocomplete_func(const char* const prefix, gboolean previous, void* context);

static char* _cmd_ac_complete_params(ProfWin* window, const char* const input, gboolean previous);

static Autocomplete commands_ac;
static Autocomplete who_room_ac;
static Autocomplete who_roster_ac;
static Autocomplete help_ac;
static Autocomplete help_commands_ac;
static Autocomplete notify_ac;
static Autocomplete notify_chat_ac;
static Autocomplete notify_room_ac;
static Autocomplete notify_typing_ac;
static Autocomplete notify_mention_ac;
static Autocomplete notify_offline_ac;
static Autocomplete notify_trigger_ac;
static Autocomplete prefs_ac;
static Autocomplete sub_ac;
static Autocomplete log_ac;
static Autocomplete log_level_ac;
static Autocomplete autoaway_ac;
static Autocomplete autoaway_mode_ac;
static Autocomplete autoaway_presence_ac;
static Autocomplete autoconnect_ac;
static Autocomplete wintitle_ac;
static Autocomplete theme_ac;
static Autocomplete theme_load_ac;
static Autocomplete account_ac;
static Autocomplete account_set_ac;
static Autocomplete account_clear_ac;
static Autocomplete account_default_ac;
static Autocomplete account_status_ac;
static Autocomplete disco_ac;
static Autocomplete wins_ac;
static Autocomplete roster_ac;
static Autocomplete roster_show_ac;
static Autocomplete roster_by_ac;
static Autocomplete roster_count_ac;
static Autocomplete roster_order_ac;
static Autocomplete roster_header_ac;
static Autocomplete roster_contact_ac;
static Autocomplete roster_resource_ac;
static Autocomplete roster_presence_ac;
static Autocomplete roster_char_ac;
static Autocomplete roster_remove_all_ac;
static Autocomplete roster_room_ac;
static Autocomplete roster_room_show_ac;
static Autocomplete roster_room_use_ac;
static Autocomplete roster_room_position_ac;
static Autocomplete roster_room_by_ac;
static Autocomplete roster_room_order_ac;
static Autocomplete roster_unread_ac;
static Autocomplete roster_private_ac;
static Autocomplete group_ac;
static Autocomplete bookmark_ac;
static Autocomplete bookmark_property_ac;
static Autocomplete bookmark_ignore_ac;
#ifdef HAVE_LIBOTR
static Autocomplete otr_ac;
static Autocomplete otr_log_ac;
static Autocomplete otr_policy_ac;
static Autocomplete otr_sendfile_ac;
#endif
#ifdef HAVE_OMEMO
static Autocomplete omemo_ac;
static Autocomplete omemo_log_ac;
static Autocomplete omemo_policy_ac;
static Autocomplete omemo_trustmode_ac;
#endif
static Autocomplete connect_property_ac;
static Autocomplete tls_property_ac;
static Autocomplete auth_property_ac;
static Autocomplete alias_ac;
static Autocomplete aliases_ac;
static Autocomplete join_property_ac;
static Autocomplete room_ac;
static Autocomplete rooms_all_ac;
static Autocomplete rooms_list_ac;
static Autocomplete rooms_cache_ac;
static Autocomplete affiliation_ac;
static Autocomplete role_ac;
static Autocomplete affiliation_cmd_ac;
static Autocomplete role_cmd_ac;
static Autocomplete subject_ac;
static Autocomplete form_ac;
static Autocomplete form_field_multi_ac;
static Autocomplete occupants_ac;
static Autocomplete occupants_default_ac;
static Autocomplete occupants_show_ac;
static Autocomplete occupants_header_ac;
static Autocomplete occupants_header_char_ac;
static Autocomplete occupants_char_ac;
static Autocomplete time_ac;
static Autocomplete time_format_ac;
static Autocomplete resource_ac;
static Autocomplete inpblock_ac;
static Autocomplete receipts_ac;
#ifdef HAVE_LIBGPGME
static Autocomplete pgp_ac;
static Autocomplete pgp_log_ac;
static Autocomplete pgp_sendfile_ac;
static Autocomplete ox_ac;
static Autocomplete ox_log_ac;
#endif
static Autocomplete tls_ac;
static Autocomplete titlebar_ac;
static Autocomplete titlebar_show_ac;
static Autocomplete tls_certpath_ac;
static Autocomplete script_ac;
static Autocomplete script_show_ac;
static Autocomplete console_ac;
static Autocomplete console_msg_ac;
static Autocomplete autoping_ac;
static Autocomplete plugins_ac;
static Autocomplete plugins_load_ac;
static Autocomplete plugins_unload_ac;
static Autocomplete plugins_reload_ac;
static Autocomplete filepath_ac;
static Autocomplete blocked_ac;
static Autocomplete tray_ac;
static Autocomplete presence_ac;
static Autocomplete presence_setting_ac;
static Autocomplete winpos_ac;
static Autocomplete statusbar_ac;
static Autocomplete statusbar_self_ac;
static Autocomplete statusbar_chat_ac;
static Autocomplete statusbar_room_ac;
static Autocomplete statusbar_show_ac;
static Autocomplete clear_ac;
static Autocomplete invite_ac;
static Autocomplete status_ac;
static Autocomplete status_state_ac;
static Autocomplete logging_ac;
static Autocomplete logging_group_ac;
static Autocomplete color_ac;
static Autocomplete correction_ac;
static Autocomplete avatar_ac;
static Autocomplete url_ac;
static Autocomplete executable_ac;
static Autocomplete intype_ac;
static Autocomplete mood_ac;
static Autocomplete mood_type_ac;
static Autocomplete strophe_ac;
static Autocomplete strophe_sm_ac;
static Autocomplete strophe_verbosity_ac;
static Autocomplete adhoc_cmd_ac;
static Autocomplete lastactivity_ac;
static Autocomplete vcard_ac;
static Autocomplete vcard_photo_ac;
static Autocomplete vcard_element_ac;
static Autocomplete vcard_set_ac;
static Autocomplete vcard_name_ac;
static Autocomplete vcard_set_param_ac;
static Autocomplete vcard_togglable_param_ac;
static Autocomplete vcard_toggle_ac;
static Autocomplete vcard_address_type_ac;

/*!
 * \brief Initialization of auto completion for commands.
 *
 * This function implements the auto completion for profanity's commands.
 *
 */

void
cmd_ac_init(void)
{
    commands_ac = autocomplete_new();
    aliases_ac = autocomplete_new();

    help_ac = autocomplete_new();
    autocomplete_add(help_ac, "commands");
    autocomplete_add(help_ac, "navigation");
    autocomplete_add(help_ac, "search_all");
    autocomplete_add(help_ac, "search_any");

    help_commands_ac = autocomplete_new();
    autocomplete_add(help_commands_ac, "chat");
    autocomplete_add(help_commands_ac, "groupchat");
    autocomplete_add(help_commands_ac, "roster");
    autocomplete_add(help_commands_ac, "presence");
    autocomplete_add(help_commands_ac, "discovery");
    autocomplete_add(help_commands_ac, "connection");
    autocomplete_add(help_commands_ac, "ui");
    autocomplete_add(help_commands_ac, "plugins");

    prefs_ac = autocomplete_new();
    autocomplete_add(prefs_ac, "ui");
    autocomplete_add(prefs_ac, "desktop");
    autocomplete_add(prefs_ac, "chat");
    autocomplete_add(prefs_ac, "log");
    autocomplete_add(prefs_ac, "conn");
    autocomplete_add(prefs_ac, "presence");
#ifdef HAVE_LIBOTR
    autocomplete_add(prefs_ac, "otr");
#endif
#ifdef HAVE_LIBGPGME
    autocomplete_add(prefs_ac, "pgp");
    autocomplete_add(prefs_ac, "ox");
#endif
#ifdef HAVE_OMEMO
    autocomplete_add(prefs_ac, "omemo");
#endif

    notify_ac = autocomplete_new();
    autocomplete_add(notify_ac, "chat");
    autocomplete_add(notify_ac, "room");
    autocomplete_add(notify_ac, "typing");
    autocomplete_add(notify_ac, "remind");
    autocomplete_add(notify_ac, "invite");
    autocomplete_add(notify_ac, "sub");
    autocomplete_add(notify_ac, "on");
    autocomplete_add(notify_ac, "off");
    autocomplete_add(notify_ac, "mention");
    autocomplete_add(notify_ac, "trigger");
    autocomplete_add(notify_ac, "reset");

    notify_chat_ac = autocomplete_new();
    autocomplete_add(notify_chat_ac, "on");
    autocomplete_add(notify_chat_ac, "off");
    autocomplete_add(notify_chat_ac, "current");
    autocomplete_add(notify_chat_ac, "text");

    notify_room_ac = autocomplete_new();
    autocomplete_add(notify_room_ac, "on");
    autocomplete_add(notify_room_ac, "off");
    autocomplete_add(notify_room_ac, "mention");
    autocomplete_add(notify_room_ac, "offline");
    autocomplete_add(notify_room_ac, "current");
    autocomplete_add(notify_room_ac, "text");
    autocomplete_add(notify_room_ac, "trigger");

    notify_typing_ac = autocomplete_new();
    autocomplete_add(notify_typing_ac, "on");
    autocomplete_add(notify_typing_ac, "off");
    autocomplete_add(notify_typing_ac, "current");

    notify_mention_ac = autocomplete_new();
    autocomplete_add(notify_mention_ac, "on");
    autocomplete_add(notify_mention_ac, "off");
    autocomplete_add(notify_mention_ac, "case_sensitive");
    autocomplete_add(notify_mention_ac, "case_insensitive");
    autocomplete_add(notify_mention_ac, "word_whole");
    autocomplete_add(notify_mention_ac, "word_part");

    notify_offline_ac = autocomplete_new();
    autocomplete_add(notify_offline_ac, "on");
    autocomplete_add(notify_offline_ac, "off");

    notify_trigger_ac = autocomplete_new();
    autocomplete_add(notify_trigger_ac, "add");
    autocomplete_add(notify_trigger_ac, "remove");
    autocomplete_add(notify_trigger_ac, "list");
    autocomplete_add(notify_trigger_ac, "on");
    autocomplete_add(notify_trigger_ac, "off");

    sub_ac = autocomplete_new();
    autocomplete_add(sub_ac, "request");
    autocomplete_add(sub_ac, "allow");
    autocomplete_add(sub_ac, "deny");
    autocomplete_add(sub_ac, "show");
    autocomplete_add(sub_ac, "sent");
    autocomplete_add(sub_ac, "received");

    wintitle_ac = autocomplete_new();
    autocomplete_add(wintitle_ac, "show");
    autocomplete_add(wintitle_ac, "goodbye");

    log_ac = autocomplete_new();
    autocomplete_add(log_ac, "maxsize");
    autocomplete_add(log_ac, "rotate");
    autocomplete_add(log_ac, "shared");
    autocomplete_add(log_ac, "where");
    autocomplete_add(log_ac, "level");

    log_level_ac = autocomplete_new();
    autocomplete_add(log_level_ac, "WARN");
    autocomplete_add(log_level_ac, "INFO");
    autocomplete_add(log_level_ac, "DEBUG");
    autocomplete_add(log_level_ac, "ERROR");

    autoaway_ac = autocomplete_new();
    autocomplete_add(autoaway_ac, "mode");
    autocomplete_add(autoaway_ac, "time");
    autocomplete_add(autoaway_ac, "message");
    autocomplete_add(autoaway_ac, "check");

    autoaway_mode_ac = autocomplete_new();
    autocomplete_add(autoaway_mode_ac, "away");
    autocomplete_add(autoaway_mode_ac, "idle");
    autocomplete_add(autoaway_mode_ac, "off");

    autoaway_presence_ac = autocomplete_new();
    autocomplete_add(autoaway_presence_ac, "away");
    autocomplete_add(autoaway_presence_ac, "xa");

    autoconnect_ac = autocomplete_new();
    autocomplete_add(autoconnect_ac, "set");
    autocomplete_add(autoconnect_ac, "off");

    theme_ac = autocomplete_new();
    autocomplete_add(theme_ac, "load");
    autocomplete_add(theme_ac, "full-load");
    autocomplete_add(theme_ac, "list");
    autocomplete_add(theme_ac, "colours");
    autocomplete_add(theme_ac, "properties");

    disco_ac = autocomplete_new();
    autocomplete_add(disco_ac, "info");
    autocomplete_add(disco_ac, "items");

    account_ac = autocomplete_new();
    autocomplete_add(account_ac, "list");
    autocomplete_add(account_ac, "show");
    autocomplete_add(account_ac, "add");
    autocomplete_add(account_ac, "remove");
    autocomplete_add(account_ac, "enable");
    autocomplete_add(account_ac, "disable");
    autocomplete_add(account_ac, "default");
    autocomplete_add(account_ac, "rename");
    autocomplete_add(account_ac, "set");
    autocomplete_add(account_ac, "clear");

    account_set_ac = autocomplete_new();
    autocomplete_add(account_set_ac, "jid");
    autocomplete_add(account_set_ac, "server");
    autocomplete_add(account_set_ac, "port");
    autocomplete_add(account_set_ac, "status");
    autocomplete_add(account_set_ac, "online");
    autocomplete_add(account_set_ac, "chat");
    autocomplete_add(account_set_ac, "away");
    autocomplete_add(account_set_ac, "xa");
    autocomplete_add(account_set_ac, "dnd");
    autocomplete_add(account_set_ac, "resource");
    autocomplete_add(account_set_ac, "password");
    autocomplete_add(account_set_ac, "eval_password");
    autocomplete_add(account_set_ac, "muc");
    autocomplete_add(account_set_ac, "nick");
    autocomplete_add(account_set_ac, "otr");
    autocomplete_add(account_set_ac, "pgpkeyid");
    autocomplete_add(account_set_ac, "startscript");
    autocomplete_add(account_set_ac, "tls");
    autocomplete_add(account_set_ac, "auth");
    autocomplete_add(account_set_ac, "theme");

    account_clear_ac = autocomplete_new();
    autocomplete_add(account_clear_ac, "password");
    autocomplete_add(account_clear_ac, "eval_password");
    autocomplete_add(account_clear_ac, "server");
    autocomplete_add(account_clear_ac, "port");
    autocomplete_add(account_clear_ac, "otr");
    autocomplete_add(account_clear_ac, "pgpkeyid");
    autocomplete_add(account_clear_ac, "startscript");
    autocomplete_add(account_clear_ac, "theme");
    autocomplete_add(account_clear_ac, "muc");
    autocomplete_add(account_clear_ac, "resource");

    account_default_ac = autocomplete_new();
    autocomplete_add(account_default_ac, "set");
    autocomplete_add(account_default_ac, "off");

    account_status_ac = autocomplete_new();
    autocomplete_add(account_status_ac, "online");
    autocomplete_add(account_status_ac, "chat");
    autocomplete_add(account_status_ac, "away");
    autocomplete_add(account_status_ac, "xa");
    autocomplete_add(account_status_ac, "dnd");
    autocomplete_add(account_status_ac, "last");

    wins_ac = autocomplete_new();
    autocomplete_add(wins_ac, "unread");
    autocomplete_add(wins_ac, "attention");
    autocomplete_add(wins_ac, "prune");
    autocomplete_add(wins_ac, "swap");

    roster_ac = autocomplete_new();
    autocomplete_add(roster_ac, "add");
    autocomplete_add(roster_ac, "online");
    autocomplete_add(roster_ac, "nick");
    autocomplete_add(roster_ac, "clearnick");
    autocomplete_add(roster_ac, "remove");
    autocomplete_add(roster_ac, "remove_all");
    autocomplete_add(roster_ac, "show");
    autocomplete_add(roster_ac, "hide");
    autocomplete_add(roster_ac, "by");
    autocomplete_add(roster_ac, "count");
    autocomplete_add(roster_ac, "color");
    autocomplete_add(roster_ac, "order");
    autocomplete_add(roster_ac, "unread");
    autocomplete_add(roster_ac, "room");
    autocomplete_add(roster_ac, "size");
    autocomplete_add(roster_ac, "wrap");
    autocomplete_add(roster_ac, "header");
    autocomplete_add(roster_ac, "contact");
    autocomplete_add(roster_ac, "resource");
    autocomplete_add(roster_ac, "presence");
    autocomplete_add(roster_ac, "private");
    autocomplete_add(roster_ac, "group");

    roster_private_ac = autocomplete_new();
    autocomplete_add(roster_private_ac, "room");
    autocomplete_add(roster_private_ac, "group");
    autocomplete_add(roster_private_ac, "off");
    autocomplete_add(roster_private_ac, "char");

    roster_header_ac = autocomplete_new();
    autocomplete_add(roster_header_ac, "char");

    roster_contact_ac = autocomplete_new();
    autocomplete_add(roster_contact_ac, "char");
    autocomplete_add(roster_contact_ac, "indent");

    roster_resource_ac = autocomplete_new();
    autocomplete_add(roster_resource_ac, "char");
    autocomplete_add(roster_resource_ac, "indent");
    autocomplete_add(roster_resource_ac, "join");

    roster_presence_ac = autocomplete_new();
    autocomplete_add(roster_presence_ac, "indent");

    roster_char_ac = autocomplete_new();
    autocomplete_add(roster_char_ac, "none");

    roster_show_ac = autocomplete_new();
    autocomplete_add(roster_show_ac, "offline");
    autocomplete_add(roster_show_ac, "resource");
    autocomplete_add(roster_show_ac, "presence");
    autocomplete_add(roster_show_ac, "status");
    autocomplete_add(roster_show_ac, "empty");
    autocomplete_add(roster_show_ac, "priority");
    autocomplete_add(roster_show_ac, "contacts");
    autocomplete_add(roster_show_ac, "unsubscribed");
    autocomplete_add(roster_show_ac, "rooms");

    roster_by_ac = autocomplete_new();
    autocomplete_add(roster_by_ac, "group");
    autocomplete_add(roster_by_ac, "presence");
    autocomplete_add(roster_by_ac, "none");

    roster_count_ac = autocomplete_new();
    autocomplete_add(roster_count_ac, "unread");
    autocomplete_add(roster_count_ac, "items");
    autocomplete_add(roster_count_ac, "off");
    autocomplete_add(roster_count_ac, "zero");

    roster_order_ac = autocomplete_new();
    autocomplete_add(roster_order_ac, "name");
    autocomplete_add(roster_order_ac, "presence");

    roster_unread_ac = autocomplete_new();
    autocomplete_add(roster_unread_ac, "before");
    autocomplete_add(roster_unread_ac, "after");
    autocomplete_add(roster_unread_ac, "off");

    roster_room_ac = autocomplete_new();
    autocomplete_add(roster_room_ac, "char");
    autocomplete_add(roster_room_ac, "position");
    autocomplete_add(roster_room_ac, "by");
    autocomplete_add(roster_room_ac, "order");
    autocomplete_add(roster_room_ac, "unread");
    autocomplete_add(roster_room_ac, "private");
    autocomplete_add(roster_room_ac, "show");
    autocomplete_add(roster_room_ac, "hide");
    autocomplete_add(roster_room_ac, "use");

    roster_room_show_ac = autocomplete_new();
    autocomplete_add(roster_room_show_ac, "server");

    roster_room_by_ac = autocomplete_new();
    autocomplete_add(roster_room_by_ac, "service");
    autocomplete_add(roster_room_by_ac, "none");

    roster_room_use_ac = autocomplete_new();
    autocomplete_add(roster_room_use_ac, "jid");
    autocomplete_add(roster_room_use_ac, "name");

    roster_room_order_ac = autocomplete_new();
    autocomplete_add(roster_room_order_ac, "name");
    autocomplete_add(roster_room_order_ac, "unread");

    roster_room_position_ac = autocomplete_new();
    autocomplete_add(roster_room_position_ac, "first");
    autocomplete_add(roster_room_position_ac, "last");

    roster_remove_all_ac = autocomplete_new();
    autocomplete_add(roster_remove_all_ac, "contacts");

    group_ac = autocomplete_new();
    autocomplete_add(group_ac, "show");
    autocomplete_add(group_ac, "add");
    autocomplete_add(group_ac, "remove");

    theme_load_ac = NULL;
    plugins_load_ac = NULL;
    plugins_unload_ac = NULL;
    plugins_reload_ac = NULL;

    who_roster_ac = autocomplete_new();
    autocomplete_add(who_roster_ac, "chat");
    autocomplete_add(who_roster_ac, "online");
    autocomplete_add(who_roster_ac, "away");
    autocomplete_add(who_roster_ac, "xa");
    autocomplete_add(who_roster_ac, "dnd");
    autocomplete_add(who_roster_ac, "offline");
    autocomplete_add(who_roster_ac, "available");
    autocomplete_add(who_roster_ac, "unavailable");
    autocomplete_add(who_roster_ac, "any");

    who_room_ac = autocomplete_new();
    autocomplete_add(who_room_ac, "chat");
    autocomplete_add(who_room_ac, "online");
    autocomplete_add(who_room_ac, "away");
    autocomplete_add(who_room_ac, "xa");
    autocomplete_add(who_room_ac, "dnd");
    autocomplete_add(who_room_ac, "available");
    autocomplete_add(who_room_ac, "unavailable");
    autocomplete_add(who_room_ac, "moderator");
    autocomplete_add(who_room_ac, "participant");
    autocomplete_add(who_room_ac, "visitor");
    autocomplete_add(who_room_ac, "owner");
    autocomplete_add(who_room_ac, "admin");
    autocomplete_add(who_room_ac, "member");
    autocomplete_add(who_room_ac, "none");

    bookmark_ac = autocomplete_new();
    autocomplete_add(bookmark_ac, "list");
    autocomplete_add(bookmark_ac, "add");
    autocomplete_add(bookmark_ac, "update");
    autocomplete_add(bookmark_ac, "remove");
    autocomplete_add(bookmark_ac, "join");
    autocomplete_add(bookmark_ac, "invites");
    autocomplete_add(bookmark_ac, "ignore");

    bookmark_property_ac = autocomplete_new();
    autocomplete_add(bookmark_property_ac, "nick");
    autocomplete_add(bookmark_property_ac, "password");
    autocomplete_add(bookmark_property_ac, "autojoin");
    autocomplete_add(bookmark_property_ac, "name");

    bookmark_ignore_ac = autocomplete_new();
    autocomplete_add(bookmark_ignore_ac, "add");
    autocomplete_add(bookmark_ignore_ac, "remove");

#ifdef HAVE_LIBOTR
    otr_ac = autocomplete_new();
    autocomplete_add(otr_ac, "gen");
    autocomplete_add(otr_ac, "start");
    autocomplete_add(otr_ac, "end");
    autocomplete_add(otr_ac, "myfp");
    autocomplete_add(otr_ac, "theirfp");
    autocomplete_add(otr_ac, "trust");
    autocomplete_add(otr_ac, "untrust");
    autocomplete_add(otr_ac, "secret");
    autocomplete_add(otr_ac, "log");
    autocomplete_add(otr_ac, "libver");
    autocomplete_add(otr_ac, "policy");
    autocomplete_add(otr_ac, "question");
    autocomplete_add(otr_ac, "answer");
    autocomplete_add(otr_ac, "char");
    autocomplete_add(otr_ac, "sendfile");

    otr_log_ac = autocomplete_new();
    autocomplete_add(otr_log_ac, "on");
    autocomplete_add(otr_log_ac, "off");
    autocomplete_add(otr_log_ac, "redact");

    otr_policy_ac = autocomplete_new();
    autocomplete_add(otr_policy_ac, "manual");
    autocomplete_add(otr_policy_ac, "opportunistic");
    autocomplete_add(otr_policy_ac, "always");

    otr_sendfile_ac = autocomplete_new();
    autocomplete_add(otr_sendfile_ac, "on");
    autocomplete_add(otr_sendfile_ac, "off");
#endif

#ifdef HAVE_OMEMO
    omemo_ac = autocomplete_new();
    autocomplete_add(omemo_ac, "gen");
    autocomplete_add(omemo_ac, "log");
    autocomplete_add(omemo_ac, "start");
    autocomplete_add(omemo_ac, "end");
    autocomplete_add(omemo_ac, "trust");
    autocomplete_add(omemo_ac, "untrust");
    autocomplete_add(omemo_ac, "fingerprint");
    autocomplete_add(omemo_ac, "clear_device_list");
    autocomplete_add(omemo_ac, "policy");
    autocomplete_add(omemo_ac, "trustmode");
    autocomplete_add(omemo_ac, "char");
    autocomplete_add(omemo_ac, "qrcode");

    omemo_log_ac = autocomplete_new();
    autocomplete_add(omemo_log_ac, "on");
    autocomplete_add(omemo_log_ac, "off");
    autocomplete_add(omemo_log_ac, "redact");

    omemo_policy_ac = autocomplete_new();
    autocomplete_add(omemo_policy_ac, "manual");
    autocomplete_add(omemo_policy_ac, "automatic");
    autocomplete_add(omemo_policy_ac, "always");

    // Autocomplete OMEMO trustmode
    omemo_trustmode_ac = autocomplete_new();
    autocomplete_add(omemo_trustmode_ac, "manual");
    autocomplete_add(omemo_trustmode_ac, "firstusage");
    autocomplete_add(omemo_trustmode_ac, "blind");
#endif

    connect_property_ac = autocomplete_new();
    autocomplete_add(connect_property_ac, "auth");
    autocomplete_add(connect_property_ac, "server");
    autocomplete_add(connect_property_ac, "port");
    autocomplete_add(connect_property_ac, "tls");

    tls_property_ac = autocomplete_new();
    autocomplete_add(tls_property_ac, "force");
    autocomplete_add(tls_property_ac, "allow");
    autocomplete_add(tls_property_ac, "trust");
    autocomplete_add(tls_property_ac, "legacy");
    autocomplete_add(tls_property_ac, "disable");

    auth_property_ac = autocomplete_new();
    autocomplete_add(auth_property_ac, "default");
    autocomplete_add(auth_property_ac, "legacy");

    join_property_ac = autocomplete_new();
    autocomplete_add(join_property_ac, "nick");
    autocomplete_add(join_property_ac, "password");

    alias_ac = autocomplete_new();
    autocomplete_add(alias_ac, "add");
    autocomplete_add(alias_ac, "remove");
    autocomplete_add(alias_ac, "list");

    room_ac = autocomplete_new();
    autocomplete_add(room_ac, "accept");
    autocomplete_add(room_ac, "destroy");
    autocomplete_add(room_ac, "config");

    rooms_all_ac = autocomplete_new();
    autocomplete_add(rooms_all_ac, "service");
    autocomplete_add(rooms_all_ac, "filter");
    autocomplete_add(rooms_all_ac, "cache");

    rooms_list_ac = autocomplete_new();
    autocomplete_add(rooms_list_ac, "service");
    autocomplete_add(rooms_list_ac, "filter");

    rooms_cache_ac = autocomplete_new();
    autocomplete_add(rooms_cache_ac, "on");
    autocomplete_add(rooms_cache_ac, "off");
    autocomplete_add(rooms_cache_ac, "clear");

    affiliation_ac = autocomplete_new();
    autocomplete_add(affiliation_ac, "owner");
    autocomplete_add(affiliation_ac, "admin");
    autocomplete_add(affiliation_ac, "member");
    autocomplete_add(affiliation_ac, "none");
    autocomplete_add(affiliation_ac, "outcast");

    role_ac = autocomplete_new();
    autocomplete_add(role_ac, "moderator");
    autocomplete_add(role_ac, "participant");
    autocomplete_add(role_ac, "visitor");
    autocomplete_add(role_ac, "none");

    affiliation_cmd_ac = autocomplete_new();
    autocomplete_add(affiliation_cmd_ac, "list");
    autocomplete_add(affiliation_cmd_ac, "request");
    autocomplete_add(affiliation_cmd_ac, "register");
    autocomplete_add(affiliation_cmd_ac, "set");

    role_cmd_ac = autocomplete_new();
    autocomplete_add(role_cmd_ac, "list");
    autocomplete_add(role_cmd_ac, "set");

    subject_ac = autocomplete_new();
    autocomplete_add(subject_ac, "set");
    autocomplete_add(subject_ac, "edit");
    autocomplete_add(subject_ac, "editor");
    autocomplete_add(subject_ac, "prepend");
    autocomplete_add(subject_ac, "append");
    autocomplete_add(subject_ac, "clear");

    form_ac = autocomplete_new();
    autocomplete_add(form_ac, "submit");
    autocomplete_add(form_ac, "cancel");
    autocomplete_add(form_ac, "show");
    autocomplete_add(form_ac, "help");

    form_field_multi_ac = autocomplete_new();
    autocomplete_add(form_field_multi_ac, "add");
    autocomplete_add(form_field_multi_ac, "remove");

    occupants_ac = autocomplete_new();
    autocomplete_add(occupants_ac, "show");
    autocomplete_add(occupants_ac, "hide");
    autocomplete_add(occupants_ac, "default");
    autocomplete_add(occupants_ac, "size");
    autocomplete_add(occupants_ac, "indent");
    autocomplete_add(occupants_ac, "header");
    autocomplete_add(occupants_ac, "wrap");
    autocomplete_add(occupants_ac, "char");
    autocomplete_add(occupants_ac, "color");

    occupants_default_ac = autocomplete_new();
    autocomplete_add(occupants_default_ac, "show");
    autocomplete_add(occupants_default_ac, "hide");

    occupants_show_ac = autocomplete_new();
    autocomplete_add(occupants_show_ac, "jid");
    autocomplete_add(occupants_show_ac, "offline");

    occupants_char_ac = autocomplete_new();
    autocomplete_add(occupants_char_ac, "none");

    occupants_header_ac = autocomplete_new();
    autocomplete_add(occupants_header_ac, "char");

    occupants_header_char_ac = autocomplete_new();
    autocomplete_add(occupants_header_char_ac, "none");

    time_ac = autocomplete_new();
    autocomplete_add(time_ac, "console");
    autocomplete_add(time_ac, "chat");
    autocomplete_add(time_ac, "muc");
    autocomplete_add(time_ac, "config");
    autocomplete_add(time_ac, "private");
    autocomplete_add(time_ac, "xml");
    autocomplete_add(time_ac, "statusbar");
    autocomplete_add(time_ac, "lastactivity");
    autocomplete_add(time_ac, "all");

    time_format_ac = autocomplete_new();
    autocomplete_add(time_format_ac, "set");
    autocomplete_add(time_format_ac, "off");

    resource_ac = autocomplete_new();
    autocomplete_add(resource_ac, "set");
    autocomplete_add(resource_ac, "off");
    autocomplete_add(resource_ac, "title");
    autocomplete_add(resource_ac, "message");

    inpblock_ac = autocomplete_new();
    autocomplete_add(inpblock_ac, "timeout");
    autocomplete_add(inpblock_ac, "dynamic");

    receipts_ac = autocomplete_new();
    autocomplete_add(receipts_ac, "send");
    autocomplete_add(receipts_ac, "request");

#ifdef HAVE_LIBGPGME
    pgp_ac = autocomplete_new();
    autocomplete_add(pgp_ac, "keys");
    autocomplete_add(pgp_ac, "contacts");
    autocomplete_add(pgp_ac, "setkey");
    autocomplete_add(pgp_ac, "libver");
    autocomplete_add(pgp_ac, "start");
    autocomplete_add(pgp_ac, "end");
    autocomplete_add(pgp_ac, "log");
    autocomplete_add(pgp_ac, "char");
    autocomplete_add(pgp_ac, "sendfile");

    pgp_log_ac = autocomplete_new();
    autocomplete_add(pgp_log_ac, "on");
    autocomplete_add(pgp_log_ac, "off");
    autocomplete_add(pgp_log_ac, "redact");

    pgp_sendfile_ac = autocomplete_new();
    autocomplete_add(pgp_sendfile_ac, "on");
    autocomplete_add(pgp_sendfile_ac, "off");

    ox_ac = autocomplete_new();
    autocomplete_add(ox_ac, "keys");
    autocomplete_add(ox_ac, "contacts");
    autocomplete_add(ox_ac, "start");
    autocomplete_add(ox_ac, "end");
    autocomplete_add(ox_ac, "log");
    autocomplete_add(ox_ac, "char");
    autocomplete_add(ox_ac, "announce");
    autocomplete_add(ox_ac, "discover");
    autocomplete_add(ox_ac, "request");

    ox_log_ac = autocomplete_new();
    autocomplete_add(ox_log_ac, "on");
    autocomplete_add(ox_log_ac, "off");
    autocomplete_add(ox_log_ac, "redact");
#endif

    tls_ac = autocomplete_new();
    autocomplete_add(tls_ac, "allow");
    autocomplete_add(tls_ac, "always");
    autocomplete_add(tls_ac, "deny");
    autocomplete_add(tls_ac, "cert");
    autocomplete_add(tls_ac, "trust");
    autocomplete_add(tls_ac, "trusted");
    autocomplete_add(tls_ac, "revoke");
    autocomplete_add(tls_ac, "certpath");

    titlebar_ac = autocomplete_new();
    autocomplete_add(titlebar_ac, "up");
    autocomplete_add(titlebar_ac, "down");
    autocomplete_add(titlebar_ac, "show");
    autocomplete_add(titlebar_ac, "hide");

    titlebar_show_ac = autocomplete_new();
    autocomplete_add(titlebar_show_ac, "tls");
    autocomplete_add(titlebar_show_ac, "encwarn");
    autocomplete_add(titlebar_show_ac, "resource");
    autocomplete_add(titlebar_show_ac, "presence");
    autocomplete_add(titlebar_show_ac, "jid");
    autocomplete_add(titlebar_show_ac, "name");

    tls_certpath_ac = autocomplete_new();
    autocomplete_add(tls_certpath_ac, "set");
    autocomplete_add(tls_certpath_ac, "clear");
    autocomplete_add(tls_certpath_ac, "default");

    script_ac = autocomplete_new();
    autocomplete_add(script_ac, "run");
    autocomplete_add(script_ac, "list");
    autocomplete_add(script_ac, "show");

    script_show_ac = NULL;

    console_ac = autocomplete_new();
    autocomplete_add(console_ac, "chat");
    autocomplete_add(console_ac, "muc");
    autocomplete_add(console_ac, "private");

    console_msg_ac = autocomplete_new();
    autocomplete_add(console_msg_ac, "all");
    autocomplete_add(console_msg_ac, "first");
    autocomplete_add(console_msg_ac, "mention");
    autocomplete_add(console_msg_ac, "none");

    autoping_ac = autocomplete_new();
    autocomplete_add(autoping_ac, "set");
    autocomplete_add(autoping_ac, "timeout");

    plugins_ac = autocomplete_new();
    autocomplete_add(plugins_ac, "install");
    autocomplete_add(plugins_ac, "update");
    autocomplete_add(plugins_ac, "uninstall");
    autocomplete_add(plugins_ac, "load");
    autocomplete_add(plugins_ac, "unload");
    autocomplete_add(plugins_ac, "reload");
    autocomplete_add(plugins_ac, "python_version");

    filepath_ac = autocomplete_new();

    blocked_ac = autocomplete_new();
    autocomplete_add(blocked_ac, "add");
    autocomplete_add(blocked_ac, "remove");
    autocomplete_add(blocked_ac, "report-abuse");
    autocomplete_add(blocked_ac, "report-spam");

    clear_ac = autocomplete_new();
    autocomplete_add(clear_ac, "persist_history");

    tray_ac = autocomplete_new();
    autocomplete_add(tray_ac, "on");
    autocomplete_add(tray_ac, "off");
    autocomplete_add(tray_ac, "read");
    autocomplete_add(tray_ac, "timer");

    presence_ac = autocomplete_new();
    autocomplete_add(presence_ac, "titlebar");
    autocomplete_add(presence_ac, "console");
    autocomplete_add(presence_ac, "chat");
    autocomplete_add(presence_ac, "room");

    presence_setting_ac = autocomplete_new();
    autocomplete_add(presence_setting_ac, "all");
    autocomplete_add(presence_setting_ac, "online");
    autocomplete_add(presence_setting_ac, "none");

    winpos_ac = autocomplete_new();
    autocomplete_add(winpos_ac, "up");
    autocomplete_add(winpos_ac, "down");

    statusbar_ac = autocomplete_new();
    autocomplete_add(statusbar_ac, "up");
    autocomplete_add(statusbar_ac, "down");
    autocomplete_add(statusbar_ac, "show");
    autocomplete_add(statusbar_ac, "hide");
    autocomplete_add(statusbar_ac, "maxtabs");
    autocomplete_add(statusbar_ac, "tablen");
    autocomplete_add(statusbar_ac, "self");
    autocomplete_add(statusbar_ac, "chat");
    autocomplete_add(statusbar_ac, "room");

    invite_ac = autocomplete_new();
    autocomplete_add(invite_ac, "send");
    autocomplete_add(invite_ac, "list");
    autocomplete_add(invite_ac, "decline");

    statusbar_self_ac = autocomplete_new();
    autocomplete_add(statusbar_self_ac, "user");
    autocomplete_add(statusbar_self_ac, "barejid");
    autocomplete_add(statusbar_self_ac, "fulljid");
    autocomplete_add(statusbar_self_ac, "off");

    statusbar_chat_ac = autocomplete_new();
    autocomplete_add(statusbar_chat_ac, "user");
    autocomplete_add(statusbar_chat_ac, "jid");

    statusbar_room_ac = autocomplete_new();
    autocomplete_add(statusbar_room_ac, "room");
    autocomplete_add(statusbar_room_ac, "jid");

    statusbar_show_ac = autocomplete_new();
    autocomplete_add(statusbar_show_ac, "name");
    autocomplete_add(statusbar_show_ac, "number");
    autocomplete_add(statusbar_show_ac, "read");

    status_ac = autocomplete_new();
    autocomplete_add(status_ac, "set");
    autocomplete_add(status_ac, "get");

    status_state_ac = autocomplete_new();
    autocomplete_add(status_state_ac, "online");
    autocomplete_add(status_state_ac, "chat");
    autocomplete_add(status_state_ac, "away");
    autocomplete_add(status_state_ac, "xa");
    autocomplete_add(status_state_ac, "dnd");

    logging_ac = autocomplete_new();
    autocomplete_add(logging_ac, "chat");
    autocomplete_add(logging_ac, "group");

    logging_group_ac = autocomplete_new();
    autocomplete_add(logging_group_ac, "on");
    autocomplete_add(logging_group_ac, "off");
    autocomplete_add(logging_group_ac, "color");

    color_ac = autocomplete_new();
    autocomplete_add(color_ac, "on");
    autocomplete_add(color_ac, "off");
    autocomplete_add(color_ac, "redgreen");
    autocomplete_add(color_ac, "blue");
    autocomplete_add(color_ac, "own");

    correction_ac = autocomplete_new();
    autocomplete_add(correction_ac, "on");
    autocomplete_add(correction_ac, "off");
    autocomplete_add(correction_ac, "char");

    avatar_ac = autocomplete_new();
    autocomplete_add(avatar_ac, "set");
    autocomplete_add(avatar_ac, "get");
    autocomplete_add(avatar_ac, "open");

    url_ac = autocomplete_new();
    autocomplete_add(url_ac, "open");
    autocomplete_add(url_ac, "save");

    executable_ac = autocomplete_new();
    autocomplete_add(executable_ac, "avatar");
    autocomplete_add(executable_ac, "urlopen");
    autocomplete_add(executable_ac, "urlsave");
    autocomplete_add(executable_ac, "editor");

    intype_ac = autocomplete_new();
    autocomplete_add(intype_ac, "console");
    autocomplete_add(intype_ac, "titlebar");

    strophe_ac = autocomplete_new();
    autocomplete_add(strophe_ac, "sm");
    autocomplete_add(strophe_ac, "verbosity");
    strophe_sm_ac = autocomplete_new();
    autocomplete_add(strophe_sm_ac, "on");
    autocomplete_add(strophe_sm_ac, "no-resend");
    autocomplete_add(strophe_sm_ac, "off");
    strophe_verbosity_ac = autocomplete_new();
    autocomplete_add(strophe_verbosity_ac, "0");
    autocomplete_add(strophe_verbosity_ac, "1");
    autocomplete_add(strophe_verbosity_ac, "2");
    autocomplete_add(strophe_verbosity_ac, "3");

    mood_ac = autocomplete_new();
    autocomplete_add(mood_ac, "set");
    autocomplete_add(mood_ac, "clear");
    autocomplete_add(mood_ac, "on");
    autocomplete_add(mood_ac, "off");
    mood_type_ac = autocomplete_new();
    autocomplete_add(mood_type_ac, "afraid");
    autocomplete_add(mood_type_ac, "amazed");
    autocomplete_add(mood_type_ac, "angry");
    autocomplete_add(mood_type_ac, "amorous");
    autocomplete_add(mood_type_ac, "annoyed");
    autocomplete_add(mood_type_ac, "anxious");
    autocomplete_add(mood_type_ac, "aroused");
    autocomplete_add(mood_type_ac, "ashamed");
    autocomplete_add(mood_type_ac, "bored");
    autocomplete_add(mood_type_ac, "brave");
    autocomplete_add(mood_type_ac, "calm");
    autocomplete_add(mood_type_ac, "cautious");
    autocomplete_add(mood_type_ac, "cold");
    autocomplete_add(mood_type_ac, "confident");
    autocomplete_add(mood_type_ac, "confused");
    autocomplete_add(mood_type_ac, "contemplative");
    autocomplete_add(mood_type_ac, "contented");
    autocomplete_add(mood_type_ac, "cranky");
    autocomplete_add(mood_type_ac, "crazy");
    autocomplete_add(mood_type_ac, "creative");
    autocomplete_add(mood_type_ac, "curious");
    autocomplete_add(mood_type_ac, "dejected");
    autocomplete_add(mood_type_ac, "depressed");
    autocomplete_add(mood_type_ac, "disappointed");
    autocomplete_add(mood_type_ac, "disgusted");
    autocomplete_add(mood_type_ac, "dismayed");
    autocomplete_add(mood_type_ac, "distracted");
    autocomplete_add(mood_type_ac, "embarrassed");
    autocomplete_add(mood_type_ac, "envious");
    autocomplete_add(mood_type_ac, "excited");
    autocomplete_add(mood_type_ac, "flirtatious");
    autocomplete_add(mood_type_ac, "frustrated");
    autocomplete_add(mood_type_ac, "grumpy");
    autocomplete_add(mood_type_ac, "guilty");
    autocomplete_add(mood_type_ac, "happy");
    autocomplete_add(mood_type_ac, "hopeful");
    autocomplete_add(mood_type_ac, "hot");
    autocomplete_add(mood_type_ac, "humbled");
    autocomplete_add(mood_type_ac, "humiliated");
    autocomplete_add(mood_type_ac, "hungry");
    autocomplete_add(mood_type_ac, "hurt");
    autocomplete_add(mood_type_ac, "impressed");
    autocomplete_add(mood_type_ac, "in_awe");
    autocomplete_add(mood_type_ac, "in_love");
    autocomplete_add(mood_type_ac, "indignant");
    autocomplete_add(mood_type_ac, "interested");
    autocomplete_add(mood_type_ac, "intoxicated");
    autocomplete_add(mood_type_ac, "invincible");
    autocomplete_add(mood_type_ac, "jealous");
    autocomplete_add(mood_type_ac, "lonely");
    autocomplete_add(mood_type_ac, "lucky");
    autocomplete_add(mood_type_ac, "mean");
    autocomplete_add(mood_type_ac, "moody");
    autocomplete_add(mood_type_ac, "nervous");
    autocomplete_add(mood_type_ac, "neutral");
    autocomplete_add(mood_type_ac, "offended");
    autocomplete_add(mood_type_ac, "outraged");
    autocomplete_add(mood_type_ac, "playful");
    autocomplete_add(mood_type_ac, "proud");
    autocomplete_add(mood_type_ac, "relaxed");
    autocomplete_add(mood_type_ac, "relieved");
    autocomplete_add(mood_type_ac, "remorseful");
    autocomplete_add(mood_type_ac, "restless");
    autocomplete_add(mood_type_ac, "sad");
    autocomplete_add(mood_type_ac, "sarcastic");
    autocomplete_add(mood_type_ac, "serious");
    autocomplete_add(mood_type_ac, "shocked");
    autocomplete_add(mood_type_ac, "shy");
    autocomplete_add(mood_type_ac, "sick");
    autocomplete_add(mood_type_ac, "sleepy");
    autocomplete_add(mood_type_ac, "spontaneous");
    autocomplete_add(mood_type_ac, "stressed");
    autocomplete_add(mood_type_ac, "strong");
    autocomplete_add(mood_type_ac, "surprised");
    autocomplete_add(mood_type_ac, "thankful");
    autocomplete_add(mood_type_ac, "thirsty");
    autocomplete_add(mood_type_ac, "tired");
    autocomplete_add(mood_type_ac, "undefined");
    autocomplete_add(mood_type_ac, "weak");
    autocomplete_add(mood_type_ac, "worried");

    adhoc_cmd_ac = autocomplete_new();
    autocomplete_add(adhoc_cmd_ac, "list");
    autocomplete_add(adhoc_cmd_ac, "exec");

    lastactivity_ac = autocomplete_new();
    autocomplete_add(lastactivity_ac, "set");
    autocomplete_add(lastactivity_ac, "get");

    vcard_ac = autocomplete_new();
    autocomplete_add(vcard_ac, "get");
    autocomplete_add(vcard_ac, "photo");
    autocomplete_add(vcard_ac, "set");
    autocomplete_add(vcard_ac, "add");
    autocomplete_add(vcard_ac, "remove");
    autocomplete_add(vcard_ac, "save");

    vcard_photo_ac = autocomplete_new();
    autocomplete_add(vcard_photo_ac, "open");
    autocomplete_add(vcard_photo_ac, "save");

    vcard_element_ac = autocomplete_new();
    autocomplete_add(vcard_element_ac, "nickname");
    autocomplete_add(vcard_element_ac, "birthday");
    autocomplete_add(vcard_element_ac, "address");
    autocomplete_add(vcard_element_ac, "tel");
    autocomplete_add(vcard_element_ac, "email");
    autocomplete_add(vcard_element_ac, "jid");
    autocomplete_add(vcard_element_ac, "title");
    autocomplete_add(vcard_element_ac, "role");
    autocomplete_add(vcard_element_ac, "note");
    autocomplete_add(vcard_element_ac, "url");

    vcard_set_ac = autocomplete_new();
    autocomplete_add(vcard_set_ac, "fullname");
    autocomplete_add(vcard_set_ac, "name");

    vcard_name_ac = autocomplete_new();
    autocomplete_add(vcard_name_ac, "family");
    autocomplete_add(vcard_name_ac, "given");
    autocomplete_add(vcard_name_ac, "middle");
    autocomplete_add(vcard_name_ac, "prefix");
    autocomplete_add(vcard_name_ac, "suffix");

    vcard_set_param_ac = autocomplete_new();
    autocomplete_add(vcard_set_param_ac, "pobox");
    autocomplete_add(vcard_set_param_ac, "extaddr");
    autocomplete_add(vcard_set_param_ac, "street");
    autocomplete_add(vcard_set_param_ac, "locality");
    autocomplete_add(vcard_set_param_ac, "region");
    autocomplete_add(vcard_set_param_ac, "pocode");
    autocomplete_add(vcard_set_param_ac, "country");
    autocomplete_add(vcard_set_param_ac, "type");
    autocomplete_add(vcard_set_param_ac, "home");
    autocomplete_add(vcard_set_param_ac, "work");
    autocomplete_add(vcard_set_param_ac, "voice");
    autocomplete_add(vcard_set_param_ac, "fax");
    autocomplete_add(vcard_set_param_ac, "pager");
    autocomplete_add(vcard_set_param_ac, "msg");
    autocomplete_add(vcard_set_param_ac, "cell");
    autocomplete_add(vcard_set_param_ac, "video");
    autocomplete_add(vcard_set_param_ac, "bbs");
    autocomplete_add(vcard_set_param_ac, "modem");
    autocomplete_add(vcard_set_param_ac, "isdn");
    autocomplete_add(vcard_set_param_ac, "pcs");
    autocomplete_add(vcard_set_param_ac, "preferred");
    autocomplete_add(vcard_set_param_ac, "x400");

    vcard_togglable_param_ac = autocomplete_new();
    autocomplete_add(vcard_togglable_param_ac, "home");
    autocomplete_add(vcard_togglable_param_ac, "work");
    autocomplete_add(vcard_togglable_param_ac, "voice");
    autocomplete_add(vcard_togglable_param_ac, "fax");
    autocomplete_add(vcard_togglable_param_ac, "pager");
    autocomplete_add(vcard_togglable_param_ac, "msg");
    autocomplete_add(vcard_togglable_param_ac, "cell");
    autocomplete_add(vcard_togglable_param_ac, "video");
    autocomplete_add(vcard_togglable_param_ac, "bbs");
    autocomplete_add(vcard_togglable_param_ac, "modem");
    autocomplete_add(vcard_togglable_param_ac, "isdn");
    autocomplete_add(vcard_togglable_param_ac, "pcs");
    autocomplete_add(vcard_togglable_param_ac, "preferred");
    autocomplete_add(vcard_togglable_param_ac, "x400");

    vcard_toggle_ac = autocomplete_new();
    autocomplete_add(vcard_toggle_ac, "on");
    autocomplete_add(vcard_toggle_ac, "off");

    vcard_address_type_ac = autocomplete_new();
    autocomplete_add(vcard_address_type_ac, "domestic");
    autocomplete_add(vcard_address_type_ac, "international");
}

void
cmd_ac_add(const char* const value)
{
    if (commands_ac == NULL) {
        return;
    }

    autocomplete_add(commands_ac, value);
}

void
cmd_ac_add_help(const char* const value)
{
    if (help_ac == NULL) {
        return;
    }

    autocomplete_add(help_ac, value);
}

void
cmd_ac_add_cmd(const Command* command)
{
    autocomplete_add(commands_ac, command->cmd);
    autocomplete_add(help_ac, command->cmd + 1);
}

void
cmd_ac_add_alias(ProfAlias* alias)
{
    GString* ac_alias = g_string_new("/");
    g_string_append(ac_alias, alias->name);
    autocomplete_add(commands_ac, ac_alias->str);
    autocomplete_add(aliases_ac, alias->name);
    g_string_free(ac_alias, TRUE);
}

void
cmd_ac_add_alias_value(char* value)
{
    if (aliases_ac == NULL) {
        return;
    }

    autocomplete_add(aliases_ac, value);
}

void
cmd_ac_remove_alias_value(char* value)
{
    if (aliases_ac == NULL) {
        return;
    }
    autocomplete_remove(aliases_ac, value);
}

void
cmd_ac_remove(const char* const value)
{
    if (commands_ac == NULL) {
        return;
    }

    autocomplete_remove(commands_ac, value);
}

void
cmd_ac_remove_help(const char* const value)
{
    if (help_ac == NULL) {
        return;
    }

    autocomplete_remove(help_ac, value);
}

gboolean
cmd_ac_exists(char* cmd)
{
    if (commands_ac == NULL) {
        return FALSE;
    }

    return autocomplete_contains(commands_ac, cmd);
}

void
cmd_ac_add_form_fields(DataForm* form)
{
    if (form == NULL) {
        return;
    }

    GList* fields = autocomplete_create_list(form->tag_ac);
    GList* curr_field = fields;
    while (curr_field) {
        GString* field_str = g_string_new("/");
        g_string_append(field_str, curr_field->data);
        cmd_ac_add(field_str->str);
        g_string_free(field_str, TRUE);
        curr_field = g_list_next(curr_field);
    }
    g_list_free_full(fields, free);
}

void
cmd_ac_remove_form_fields(DataForm* form)
{
    if (form == NULL) {
        return;
    }

    GList* fields = autocomplete_create_list(form->tag_ac);
    GList* curr_field = fields;
    while (curr_field) {
        GString* field_str = g_string_new("/");
        g_string_append(field_str, curr_field->data);
        cmd_ac_remove(field_str->str);
        g_string_free(field_str, TRUE);
        curr_field = g_list_next(curr_field);
    }
    g_list_free_full(fields, free);
}

char*
cmd_ac_complete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;
    // autocomplete command
    if ((strncmp(input, "/", 1) == 0) && (!strchr(input, ' '))) {
        found = autocomplete_complete(commands_ac, input, TRUE, previous);

        // autocomplete parameters
    } else {
        found = _cmd_ac_complete_params(window, input, previous);
    }

    return found;
}

void
cmd_ac_reset(ProfWin* window)
{
    jabber_conn_status_t conn_status = connection_get_status();
    if (conn_status == JABBER_CONNECTED) {
        roster_reset_search_attempts();

        if (window->type == WIN_CHAT) {
            ProfChatWin* chatwin = (ProfChatWin*)window;
            assert(chatwin->memcheck == PROFCHATWIN_MEMCHECK);
            PContact contact = roster_get_contact(chatwin->barejid);
            if (contact) {
                p_contact_resource_ac_reset(contact);
            }
        }
    }

    muc_invites_reset_ac();
    muc_confserver_reset_ac();
    accounts_reset_all_search();
    accounts_reset_enabled_search();
    tlscerts_reset_ac();
    prefs_reset_boolean_choice();
    presence_reset_sub_request_search();
#ifdef HAVE_LIBGPGME
    p_gpg_autocomplete_key_reset();
#endif
#ifdef HAVE_OMEMO
    omemo_fingerprint_autocomplete_reset();
#endif
    autocomplete_reset(help_ac);
    autocomplete_reset(help_commands_ac);
    autocomplete_reset(notify_ac);
    autocomplete_reset(notify_chat_ac);
    autocomplete_reset(notify_room_ac);
    autocomplete_reset(notify_typing_ac);
    autocomplete_reset(notify_mention_ac);
    autocomplete_reset(notify_trigger_ac);
    autocomplete_reset(sub_ac);
    autocomplete_reset(filepath_ac);

    autocomplete_reset(who_room_ac);
    autocomplete_reset(who_roster_ac);
    autocomplete_reset(prefs_ac);
    autocomplete_reset(log_ac);
    autocomplete_reset(log_level_ac);
    autocomplete_reset(commands_ac);
    autocomplete_reset(autoaway_ac);
    autocomplete_reset(autoaway_mode_ac);
    autocomplete_reset(autoaway_presence_ac);
    autocomplete_reset(autoconnect_ac);
    autocomplete_reset(theme_ac);
    if (theme_load_ac) {
        autocomplete_free(theme_load_ac);
        theme_load_ac = NULL;
    }
    if (plugins_load_ac) {
        autocomplete_free(plugins_load_ac);
        plugins_load_ac = NULL;
    }
    if (plugins_unload_ac) {
        autocomplete_free(plugins_unload_ac);
        plugins_unload_ac = NULL;
    }
    if (plugins_reload_ac) {
        autocomplete_free(plugins_reload_ac);
        plugins_reload_ac = NULL;
    }
    autocomplete_reset(account_ac);
    autocomplete_reset(account_set_ac);
    autocomplete_reset(account_clear_ac);
    autocomplete_reset(account_default_ac);
    autocomplete_reset(account_status_ac);
    autocomplete_reset(disco_ac);
    autocomplete_reset(wins_ac);
    autocomplete_reset(roster_ac);
    autocomplete_reset(roster_header_ac);
    autocomplete_reset(roster_contact_ac);
    autocomplete_reset(roster_resource_ac);
    autocomplete_reset(roster_presence_ac);
    autocomplete_reset(roster_char_ac);
    autocomplete_reset(roster_show_ac);
    autocomplete_reset(roster_by_ac);
    autocomplete_reset(roster_count_ac);
    autocomplete_reset(roster_order_ac);
    autocomplete_reset(roster_room_ac);
    autocomplete_reset(roster_room_show_ac);
    autocomplete_reset(roster_room_use_ac);
    autocomplete_reset(roster_room_by_ac);
    autocomplete_reset(roster_unread_ac);
    autocomplete_reset(roster_room_position_ac);
    autocomplete_reset(roster_room_order_ac);
    autocomplete_reset(roster_remove_all_ac);
    autocomplete_reset(roster_private_ac);
    autocomplete_reset(group_ac);
    autocomplete_reset(wintitle_ac);
    autocomplete_reset(bookmark_ac);
    autocomplete_reset(bookmark_property_ac);
    autocomplete_reset(bookmark_ignore_ac);
#ifdef HAVE_LIBOTR
    autocomplete_reset(otr_ac);
    autocomplete_reset(otr_log_ac);
    autocomplete_reset(otr_policy_ac);
    autocomplete_reset(otr_sendfile_ac);
#endif
#ifdef HAVE_OMEMO
    autocomplete_reset(omemo_ac);
    autocomplete_reset(omemo_log_ac);
    autocomplete_reset(omemo_policy_ac);
    autocomplete_reset(omemo_trustmode_ac);
#endif
    autocomplete_reset(connect_property_ac);
    autocomplete_reset(tls_property_ac);
    autocomplete_reset(auth_property_ac);
    autocomplete_reset(alias_ac);
    autocomplete_reset(aliases_ac);
    autocomplete_reset(join_property_ac);
    autocomplete_reset(room_ac);
    autocomplete_reset(rooms_all_ac);
    autocomplete_reset(rooms_list_ac);
    autocomplete_reset(rooms_cache_ac);
    autocomplete_reset(affiliation_ac);
    autocomplete_reset(role_ac);
    autocomplete_reset(affiliation_cmd_ac);
    autocomplete_reset(role_cmd_ac);
    autocomplete_reset(subject_ac);
    autocomplete_reset(form_ac);
    autocomplete_reset(form_field_multi_ac);
    autocomplete_reset(occupants_ac);
    autocomplete_reset(occupants_char_ac);
    autocomplete_reset(occupants_default_ac);
    autocomplete_reset(occupants_show_ac);
    autocomplete_reset(occupants_header_ac);
    autocomplete_reset(occupants_header_char_ac);
    autocomplete_reset(time_ac);
    autocomplete_reset(time_format_ac);
    autocomplete_reset(resource_ac);
    autocomplete_reset(inpblock_ac);
    autocomplete_reset(receipts_ac);
#ifdef HAVE_LIBGPGME
    autocomplete_reset(pgp_ac);
    autocomplete_reset(pgp_log_ac);
    autocomplete_reset(pgp_sendfile_ac);
    autocomplete_reset(ox_ac);
    autocomplete_reset(ox_log_ac);
#endif
    autocomplete_reset(tls_ac);
    autocomplete_reset(titlebar_ac);
    autocomplete_reset(titlebar_show_ac);
    autocomplete_reset(tls_certpath_ac);
    autocomplete_reset(console_ac);
    autocomplete_reset(console_msg_ac);
    autocomplete_reset(autoping_ac);
    autocomplete_reset(plugins_ac);
    autocomplete_reset(blocked_ac);
    autocomplete_reset(tray_ac);
    autocomplete_reset(presence_ac);
    autocomplete_reset(presence_setting_ac);
    autocomplete_reset(winpos_ac);
    autocomplete_reset(statusbar_ac);
    autocomplete_reset(statusbar_self_ac);
    autocomplete_reset(statusbar_chat_ac);
    autocomplete_reset(statusbar_room_ac);
    autocomplete_reset(statusbar_show_ac);
    autocomplete_reset(clear_ac);
    autocomplete_reset(invite_ac);
    autocomplete_reset(status_ac);
    autocomplete_reset(status_state_ac);
    autocomplete_reset(logging_ac);
    autocomplete_reset(logging_group_ac);
    autocomplete_reset(color_ac);
    autocomplete_reset(correction_ac);
    autocomplete_reset(avatar_ac);
    autocomplete_reset(url_ac);
    autocomplete_reset(executable_ac);
    autocomplete_reset(intype_ac);
    autocomplete_reset(mood_ac);
    autocomplete_reset(mood_type_ac);
    autocomplete_reset(strophe_verbosity_ac);
    autocomplete_reset(strophe_sm_ac);
    autocomplete_reset(strophe_ac);
    autocomplete_reset(adhoc_cmd_ac);

    autocomplete_reset(vcard_ac);
    autocomplete_reset(vcard_photo_ac);
    autocomplete_reset(vcard_element_ac);
    autocomplete_reset(vcard_set_ac);
    autocomplete_reset(vcard_name_ac);
    autocomplete_reset(vcard_set_param_ac);
    autocomplete_reset(vcard_togglable_param_ac);
    autocomplete_reset(vcard_toggle_ac);
    autocomplete_reset(vcard_address_type_ac);

    autocomplete_reset(script_ac);
    autocomplete_reset(lastactivity_ac);

    if (script_show_ac) {
        autocomplete_free(script_show_ac);
        script_show_ac = NULL;
    }

    if (window->type == WIN_MUC) {
        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
        muc_autocomplete_reset(mucwin->roomjid);
        muc_jid_autocomplete_reset(mucwin->roomjid);
    }

    if (window->type == WIN_CONFIG) {
        ProfConfWin* confwin = (ProfConfWin*)window;
        assert(confwin->memcheck == PROFCONFWIN_MEMCHECK);
        if (confwin->form) {
            form_reset_autocompleters(confwin->form);
        }
    }

    bookmark_autocomplete_reset();
    blocked_ac_reset();
    prefs_reset_room_trigger_ac();
    win_reset_search_attempts();
    win_close_reset_search_attempts();
    plugins_reset_autocomplete();
}

void
cmd_ac_uninit(void)
{
    autocomplete_free(commands_ac);
    autocomplete_free(who_room_ac);
    autocomplete_free(who_roster_ac);
    autocomplete_free(help_ac);
    autocomplete_free(help_commands_ac);
    autocomplete_free(notify_ac);
    autocomplete_free(notify_chat_ac);
    autocomplete_free(notify_room_ac);
    autocomplete_free(notify_typing_ac);
    autocomplete_free(notify_mention_ac);
    autocomplete_free(notify_trigger_ac);
    autocomplete_free(sub_ac);
    autocomplete_free(wintitle_ac);
    autocomplete_free(log_ac);
    autocomplete_free(log_level_ac);
    autocomplete_free(prefs_ac);
    autocomplete_free(autoaway_ac);
    autocomplete_free(autoaway_mode_ac);
    autocomplete_free(autoaway_presence_ac);
    autocomplete_free(autoconnect_ac);
    autocomplete_free(theme_ac);
    autocomplete_free(theme_load_ac);
    autocomplete_free(account_ac);
    autocomplete_free(account_set_ac);
    autocomplete_free(account_clear_ac);
    autocomplete_free(account_default_ac);
    autocomplete_free(account_status_ac);
    autocomplete_free(disco_ac);
    autocomplete_free(wins_ac);
    autocomplete_free(roster_ac);
    autocomplete_free(roster_header_ac);
    autocomplete_free(roster_contact_ac);
    autocomplete_free(roster_resource_ac);
    autocomplete_free(roster_presence_ac);
    autocomplete_free(roster_char_ac);
    autocomplete_free(roster_show_ac);
    autocomplete_free(roster_by_ac);
    autocomplete_free(roster_count_ac);
    autocomplete_free(roster_order_ac);
    autocomplete_free(roster_room_ac);
    autocomplete_free(roster_room_show_ac);
    autocomplete_free(roster_room_use_ac);
    autocomplete_free(roster_room_by_ac);
    autocomplete_free(roster_unread_ac);
    autocomplete_free(roster_room_position_ac);
    autocomplete_free(roster_room_order_ac);
    autocomplete_free(roster_remove_all_ac);
    autocomplete_free(roster_private_ac);
    autocomplete_free(group_ac);
    autocomplete_free(bookmark_ac);
    autocomplete_free(bookmark_property_ac);
    autocomplete_free(bookmark_ignore_ac);
#ifdef HAVE_LIBOTR
    autocomplete_free(otr_ac);
    autocomplete_free(otr_log_ac);
    autocomplete_free(otr_policy_ac);
    autocomplete_free(otr_sendfile_ac);
#endif
#ifdef HAVE_OMEMO
    autocomplete_free(omemo_ac);
    autocomplete_free(omemo_log_ac);
    autocomplete_free(omemo_policy_ac);
    autocomplete_free(omemo_trustmode_ac);
#endif
    autocomplete_free(connect_property_ac);
    autocomplete_free(tls_property_ac);
    autocomplete_free(auth_property_ac);
    autocomplete_free(alias_ac);
    autocomplete_free(aliases_ac);
    autocomplete_free(join_property_ac);
    autocomplete_free(room_ac);
    autocomplete_free(rooms_all_ac);
    autocomplete_free(rooms_list_ac);
    autocomplete_free(rooms_cache_ac);
    autocomplete_free(affiliation_ac);
    autocomplete_free(role_ac);
    autocomplete_free(affiliation_cmd_ac);
    autocomplete_free(role_cmd_ac);
    autocomplete_free(subject_ac);
    autocomplete_free(form_ac);
    autocomplete_free(form_field_multi_ac);
    autocomplete_free(occupants_ac);
    autocomplete_free(occupants_char_ac);
    autocomplete_free(occupants_default_ac);
    autocomplete_free(occupants_show_ac);
    autocomplete_free(occupants_header_ac);
    autocomplete_free(time_ac);
    autocomplete_free(time_format_ac);
    autocomplete_free(resource_ac);
    autocomplete_free(inpblock_ac);
    autocomplete_free(receipts_ac);
#ifdef HAVE_LIBGPGME
    autocomplete_free(pgp_ac);
    autocomplete_free(pgp_log_ac);
    autocomplete_free(pgp_sendfile_ac);
    autocomplete_free(ox_ac);
    autocomplete_free(ox_log_ac);
#endif
    autocomplete_free(tls_ac);
    autocomplete_free(titlebar_ac);
    autocomplete_free(titlebar_show_ac);
    autocomplete_free(tls_certpath_ac);
    autocomplete_free(script_ac);
    autocomplete_free(script_show_ac);
    autocomplete_free(console_ac);
    autocomplete_free(console_msg_ac);
    autocomplete_free(autoping_ac);
    autocomplete_free(plugins_ac);
    autocomplete_free(plugins_load_ac);
    autocomplete_free(plugins_unload_ac);
    autocomplete_free(plugins_reload_ac);
    autocomplete_free(filepath_ac);
    autocomplete_free(blocked_ac);
    autocomplete_free(tray_ac);
    autocomplete_free(presence_ac);
    autocomplete_free(presence_setting_ac);
    autocomplete_free(winpos_ac);
    autocomplete_free(statusbar_ac);
    autocomplete_free(statusbar_self_ac);
    autocomplete_free(statusbar_chat_ac);
    autocomplete_free(statusbar_room_ac);
    autocomplete_free(statusbar_show_ac);
    autocomplete_free(clear_ac);
    autocomplete_free(invite_ac);
    autocomplete_free(status_ac);
    autocomplete_free(status_state_ac);
    autocomplete_free(logging_ac);
    autocomplete_free(logging_group_ac);
    autocomplete_free(color_ac);
    autocomplete_free(correction_ac);
    autocomplete_free(avatar_ac);
    autocomplete_free(url_ac);
    autocomplete_free(executable_ac);
    autocomplete_free(intype_ac);
    autocomplete_free(adhoc_cmd_ac);
    autocomplete_free(lastactivity_ac);
    autocomplete_free(vcard_ac);
    autocomplete_free(vcard_photo_ac);
    autocomplete_free(vcard_element_ac);
    autocomplete_free(vcard_set_ac);
    autocomplete_free(vcard_name_ac);
    autocomplete_free(vcard_set_param_ac);
    autocomplete_free(vcard_togglable_param_ac);
    autocomplete_free(vcard_toggle_ac);
    autocomplete_free(vcard_address_type_ac);
}

static void
_filepath_item_free(char** ptr)
{
    char* item = *ptr;
    free(item);
}

char*
cmd_ac_complete_filepath(const char* const input, char* const startstr, gboolean previous)
{
    unsigned int output_off = 0;
    char* tmp;

    // strip command
    char* inpcp = (char*)input + strlen(startstr);
    while (*inpcp == ' ') {
        inpcp++;
    }

    inpcp = strdup(inpcp);

    // strip quotes
    if (*inpcp == '"') {
        tmp = strchr(inpcp + 1, '"');
        if (tmp) {
            *tmp = '\0';
        }
        tmp = strdup(inpcp + 1);
        free(inpcp);
        inpcp = tmp;
    }

    // expand ~ to $HOME
    if (inpcp[0] == '~' && inpcp[1] == '/') {
        tmp = g_strdup_printf("%s/%sfoo", getenv("HOME"), inpcp + 2);
        if (!tmp) {
            free(inpcp);
            return NULL;
        }
        output_off = strlen(getenv("HOME")) + 1;
    } else {
        tmp = g_strdup_printf("%sfoo", inpcp);
        if (!tmp) {
            free(inpcp);
            return NULL;
        }
    }
    free(inpcp);
    inpcp = tmp;

    char* inpcp2 = strdup(inpcp);
    char* foofile = strdup(basename(inpcp2));
    char* directory = strdup(dirname(inpcp));
    free(inpcp);
    free(inpcp2);

    GArray* files = g_array_new(TRUE, FALSE, sizeof(char*));
    g_array_set_clear_func(files, (GDestroyNotify)_filepath_item_free);

    DIR* d = opendir(directory);
    if (d) {
        struct dirent* dir;

        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0) {
                continue;
            } else if (strcmp(dir->d_name, "..") == 0) {
                continue;
            } else if (*(dir->d_name) == '.' && *foofile != '.') {
                // only show hidden files on explicit request
                continue;
            }

            char* acstring;
            if (output_off) {
                tmp = g_strdup_printf("%s/%s", directory, dir->d_name);
                if (!tmp) {
                    free(directory);
                    free(foofile);
                    return NULL;
                }
                acstring = g_strdup_printf("~/%s", tmp + output_off);
                if (!acstring) {
                    free(directory);
                    free(foofile);
                    return NULL;
                }
                free(tmp);
            } else if (strcmp(directory, "/") == 0) {
                acstring = g_strdup_printf("/%s", dir->d_name);
                if (!acstring) {
                    free(directory);
                    free(foofile);
                    return NULL;
                }
            } else {
                acstring = g_strdup_printf("%s/%s", directory, dir->d_name);
                if (!acstring) {
                    free(directory);
                    free(foofile);
                    return NULL;
                }
            }

            char* acstring_cpy = strdup(acstring);
            g_array_append_val(files, acstring_cpy);
            free(acstring);
        }
        closedir(d);
    }
    free(directory);
    free(foofile);

    autocomplete_update(filepath_ac, (char**)files->data);
    g_array_free(files, TRUE);

    return autocomplete_param_with_ac(input, startstr, filepath_ac, TRUE, previous);
}

static char*
_cmd_ac_complete_params(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    jabber_conn_status_t conn_status = connection_get_status();

    // autocomplete boolean settings
    gchar* boolean_choices[] = { "/beep", "/states", "/outtype", "/flash", "/splash",
                                 "/history", "/vercheck", "/privileges", "/wrap",
                                 "/carbons", "/os", "/slashguard", "/mam", "/silence" };

    for (int i = 0; i < ARRAY_SIZE(boolean_choices); i++) {
        result = autocomplete_param_with_func(input, boolean_choices[i], prefs_autocomplete_boolean_choice, previous, NULL);
        if (result) {
            return result;
        }
    }

    // autocomplete nickname in chat rooms
    if (window->type == WIN_MUC) {
        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
        Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);
        if (nick_ac) {
            gchar* nick_choices[] = { "/msg", "/info", "/caps" };

            // Remove quote character before and after names when doing autocomplete
            char* unquoted = strip_arg_quotes(input);
            for (int i = 0; i < ARRAY_SIZE(nick_choices); i++) {
                result = autocomplete_param_with_ac(unquoted, nick_choices[i], nick_ac, TRUE, previous);
                if (result) {
                    free(unquoted);
                    return result;
                }
            }
            free(unquoted);
        }

        // otherwise autocomplete using roster
    } else if (conn_status == JABBER_CONNECTED) {
        gchar* contact_choices[] = { "/msg", "/info" };
        // Remove quote character before and after names when doing autocomplete
        char* unquoted = strip_arg_quotes(input);
        for (int i = 0; i < ARRAY_SIZE(contact_choices); i++) {
            result = autocomplete_param_with_func(unquoted, contact_choices[i], roster_contact_autocomplete, previous, NULL);
            if (result) {
                free(unquoted);
                return result;
            }
            result = autocomplete_param_with_func(unquoted, contact_choices[i], roster_barejid_autocomplete, previous, NULL);
            if (result) {
                free(unquoted);
                return result;
            }
        }
        free(unquoted);

        gchar* resource_choices[] = { "/caps", "/ping" };
        for (int i = 0; i < ARRAY_SIZE(resource_choices); i++) {
            result = autocomplete_param_with_func(input, resource_choices[i], roster_fulljid_autocomplete, previous, NULL);
            if (result) {
                return result;
            }
        }
    }

    gchar* invite_choices[] = { "/join" };
    for (int i = 0; i < ARRAY_SIZE(invite_choices); i++) {
        result = autocomplete_param_with_func(input, invite_choices[i], muc_invites_find, previous, NULL);
        if (result) {
            return result;
        }
    }

    gchar* cmds[] = { "/prefs", "/disco", "/room", "/autoping", "/mainwin", "/inputwin" };
    Autocomplete completers[] = { prefs_ac, disco_ac, room_ac, autoping_ac, winpos_ac, winpos_ac };

    for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
        result = autocomplete_param_with_ac(input, cmds[i], completers[i], TRUE, previous);
        if (result) {
            return result;
        }
    }

    GHashTable* ac_funcs = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(ac_funcs, "/help", _help_autocomplete);
    g_hash_table_insert(ac_funcs, "/who", _who_autocomplete);
    g_hash_table_insert(ac_funcs, "/sub", _sub_autocomplete);
    g_hash_table_insert(ac_funcs, "/notify", _notify_autocomplete);
    g_hash_table_insert(ac_funcs, "/autoaway", _autoaway_autocomplete);
    g_hash_table_insert(ac_funcs, "/theme", _theme_autocomplete);
    g_hash_table_insert(ac_funcs, "/log", _log_autocomplete);
    g_hash_table_insert(ac_funcs, "/account", _account_autocomplete);
    g_hash_table_insert(ac_funcs, "/roster", _roster_autocomplete);
    g_hash_table_insert(ac_funcs, "/bookmark", _bookmark_autocomplete);
    g_hash_table_insert(ac_funcs, "/autoconnect", _autoconnect_autocomplete);
#ifdef HAVE_LIBOTR
    g_hash_table_insert(ac_funcs, "/otr", _otr_autocomplete);
#endif
#ifdef HAVE_LIBGPGME
    g_hash_table_insert(ac_funcs, "/pgp", _pgp_autocomplete);
    g_hash_table_insert(ac_funcs, "/ox", _ox_autocomplete);
#endif
#ifdef HAVE_OMEMO
    g_hash_table_insert(ac_funcs, "/omemo", _omemo_autocomplete);
#endif
    g_hash_table_insert(ac_funcs, "/connect", _connect_autocomplete);
    g_hash_table_insert(ac_funcs, "/alias", _alias_autocomplete);
    g_hash_table_insert(ac_funcs, "/join", _join_autocomplete);
    g_hash_table_insert(ac_funcs, "/form", _form_autocomplete);
    g_hash_table_insert(ac_funcs, "/occupants", _occupants_autocomplete);
    g_hash_table_insert(ac_funcs, "/kick", _kick_autocomplete);
    g_hash_table_insert(ac_funcs, "/ban", _ban_autocomplete);
    g_hash_table_insert(ac_funcs, "/affiliation", _affiliation_autocomplete);
    g_hash_table_insert(ac_funcs, "/role", _role_autocomplete);
    g_hash_table_insert(ac_funcs, "/resource", _resource_autocomplete);
    g_hash_table_insert(ac_funcs, "/wintitle", _wintitle_autocomplete);
    g_hash_table_insert(ac_funcs, "/inpblock", _inpblock_autocomplete);
    g_hash_table_insert(ac_funcs, "/time", _time_autocomplete);
    g_hash_table_insert(ac_funcs, "/receipts", _receipts_autocomplete);
    g_hash_table_insert(ac_funcs, "/wins", _wins_autocomplete);
    g_hash_table_insert(ac_funcs, "/tls", _tls_autocomplete);
    g_hash_table_insert(ac_funcs, "/titlebar", _titlebar_autocomplete);
    g_hash_table_insert(ac_funcs, "/script", _script_autocomplete);
    g_hash_table_insert(ac_funcs, "/subject", _subject_autocomplete);
    g_hash_table_insert(ac_funcs, "/console", _console_autocomplete);
    g_hash_table_insert(ac_funcs, "/win", _win_autocomplete);
    g_hash_table_insert(ac_funcs, "/close", _close_autocomplete);
    g_hash_table_insert(ac_funcs, "/plugins", _plugins_autocomplete);
    g_hash_table_insert(ac_funcs, "/sendfile", _sendfile_autocomplete);
    g_hash_table_insert(ac_funcs, "/blocked", _blocked_autocomplete);
    g_hash_table_insert(ac_funcs, "/tray", _tray_autocomplete);
    g_hash_table_insert(ac_funcs, "/presence", _presence_autocomplete);
    g_hash_table_insert(ac_funcs, "/rooms", _rooms_autocomplete);
    g_hash_table_insert(ac_funcs, "/statusbar", _statusbar_autocomplete);
    g_hash_table_insert(ac_funcs, "/clear", _clear_autocomplete);
    g_hash_table_insert(ac_funcs, "/invite", _invite_autocomplete);
    g_hash_table_insert(ac_funcs, "/status", _status_autocomplete);
    g_hash_table_insert(ac_funcs, "/logging", _logging_autocomplete);
    g_hash_table_insert(ac_funcs, "/color", _color_autocomplete);
    g_hash_table_insert(ac_funcs, "/avatar", _avatar_autocomplete);
    g_hash_table_insert(ac_funcs, "/correction", _correction_autocomplete);
    g_hash_table_insert(ac_funcs, "/correct", _correct_autocomplete);
    g_hash_table_insert(ac_funcs, "/software", _software_autocomplete);
    g_hash_table_insert(ac_funcs, "/url", _url_autocomplete);
    g_hash_table_insert(ac_funcs, "/executable", _executable_autocomplete);
    g_hash_table_insert(ac_funcs, "/lastactivity", _lastactivity_autocomplete);
    g_hash_table_insert(ac_funcs, "/intype", _intype_autocomplete);
    g_hash_table_insert(ac_funcs, "/mood", _mood_autocomplete);
    g_hash_table_insert(ac_funcs, "/strophe", _strophe_autocomplete);
    g_hash_table_insert(ac_funcs, "/cmd", _adhoc_cmd_autocomplete);
    g_hash_table_insert(ac_funcs, "/vcard", _vcard_autocomplete);

    int len = strlen(input);
    char parsed[len + 1];
    int i = 0;
    while (i < len) {
        if (input[i] == ' ') {
            break;
        } else {
            parsed[i] = input[i];
        }
        i++;
    }
    parsed[i] = '\0';

    char* (*ac_func)(ProfWin*, const char* const, gboolean) = g_hash_table_lookup(ac_funcs, parsed);
    if (ac_func) {
        result = ac_func(window, input, previous);
        if (result) {
            g_hash_table_destroy(ac_funcs);
            return result;
        }
    }
    g_hash_table_destroy(ac_funcs);

    result = plugins_autocomplete(input, previous);
    if (result) {
        return result;
    }

    if (g_str_has_prefix(input, "/field")) {
        result = _form_field_autocomplete(window, input, previous);
        if (result) {
            return result;
        }
    }

    return NULL;
}

static char*
_sub_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;
    result = autocomplete_param_with_func(input, "/sub allow", presence_sub_request_find, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/sub deny", presence_sub_request_find, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/sub", sub_ac, TRUE, previous);
    return result;
}

static char*
_tray_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;
    result = autocomplete_param_with_func(input, "/tray read", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/tray", tray_ac, FALSE, previous);
    return result;
}

static char*
_who_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (window->type == WIN_MUC) {
        result = autocomplete_param_with_ac(input, "/who", who_room_ac, TRUE, previous);
        if (result) {
            return result;
        }
    } else {
        jabber_conn_status_t conn_status = connection_get_status();
        if (conn_status == JABBER_CONNECTED) {
            gchar* group_commands[] = { "/who any", "/who online", "/who offline",
                                        "/who chat", "/who away", "/who xa", "/who dnd", "/who available",
                                        "/who unavailable" };

            for (int i = 0; i < ARRAY_SIZE(group_commands); i++) {
                result = autocomplete_param_with_func(input, group_commands[i], roster_group_autocomplete, previous, NULL);
                if (result) {
                    return result;
                }
            }
        }

        result = autocomplete_param_with_ac(input, "/who", who_roster_ac, TRUE, previous);
    }

    return result;
}

static char*
_roster_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;
    result = autocomplete_param_with_ac(input, "/roster room private char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room private", roster_header_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster header char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster contact char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster private char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster resource char", roster_char_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/roster resource join", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room position", roster_room_position_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room by", roster_room_by_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room order", roster_room_order_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room unread", roster_unread_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room show", roster_room_show_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room hide", roster_room_show_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room use", roster_room_use_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/roster count zero", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/roster color", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    jabber_conn_status_t conn_status = connection_get_status();
    if (conn_status == JABBER_CONNECTED) {
        result = autocomplete_param_with_func(input, "/roster nick", roster_barejid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_with_func(input, "/roster clearnick", roster_barejid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_with_func(input, "/roster remove", roster_barejid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }

        result = autocomplete_param_with_func(input, "/roster group show", roster_group_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_no_with_func(input, "/roster group add", 5, roster_contact_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_no_with_func(input, "/roster group remove", 5, roster_contact_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_with_func(input, "/roster group add", roster_group_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_with_func(input, "/roster group remove", roster_group_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/roster remove_all", roster_remove_all_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster show", roster_show_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster hide", roster_show_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster by", roster_by_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster count", roster_count_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster order", roster_order_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster unread", roster_unread_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster room", roster_room_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/roster wrap", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster header", roster_header_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster contact", roster_contact_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster resource", roster_resource_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster presence", roster_presence_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster private", roster_private_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster group", group_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/roster", roster_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_blocked_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/blocked remove", blocked_ac_find, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/blocked", blocked_ac, FALSE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_bookmark_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    gboolean result;
    auto_gcharv gchar** args = parse_args(input, 2, 8, &result);

    if (result && ((strcmp(args[0], "add") == 0) || (strcmp(args[0], "update") == 0))) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        if ((num_args == 2 && space_at_end) || (num_args == 3 && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            found = autocomplete_param_with_ac(input, beginning->str, bookmark_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "autojoin") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "autojoin") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_func(input, beginning->str, prefs_autocomplete_boolean_choice, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 4 && space_at_end) || (num_args == 5 && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s %s %s", args[0], args[1], args[2], args[3]);
            found = autocomplete_param_with_ac(input, beginning->str, bookmark_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 5 && space_at_end && (g_strcmp0(args[4], "autojoin") == 0))
            || (num_args == 6 && (g_strcmp0(args[4], "autojoin") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4]);
            found = autocomplete_param_with_func(input, beginning->str, prefs_autocomplete_boolean_choice, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 6 && space_at_end) || (num_args == 7 && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5]);
            found = autocomplete_param_with_ac(input, beginning->str, bookmark_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 7 && space_at_end && (g_strcmp0(args[6], "autojoin") == 0))
            || (num_args == 8 && (g_strcmp0(args[6], "autojoin") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/bookmark");
            g_string_append_printf(beginning, " %s %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
            found = autocomplete_param_with_func(input, beginning->str, prefs_autocomplete_boolean_choice, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
    }

    found = autocomplete_param_with_func(input, "/bookmark remove", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark join", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark update", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark invites", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/bookmark ignore", bookmark_ignore_ac, TRUE, previous);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark ignore add", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark ignore remove", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }
    found = autocomplete_param_with_func(input, "/bookmark list", bookmark_find, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/bookmark", bookmark_ac, TRUE, previous);
    return found;
}

static char*
_notify_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/notify room trigger remove", prefs_autocomplete_room_trigger, previous, NULL);
    if (result) {
        return result;
    }

    gchar* boolean_choices1[] = { "/notify room current", "/notify chat current", "/notify typing current",
                                  "/notify room text", "/notify chat text" };
    for (int i = 0; i < ARRAY_SIZE(boolean_choices1); i++) {
        result = autocomplete_param_with_func(input, boolean_choices1[i], prefs_autocomplete_boolean_choice, previous, NULL);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/notify room mention", notify_mention_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/notify room offline", notify_offline_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/notify room trigger", notify_trigger_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/notify room", notify_room_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/notify chat", notify_chat_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/notify typing", notify_typing_ac, TRUE, previous);
    if (result) {
        return result;
    }

    gchar* boolean_choices2[] = { "/notify invite", "/notify sub", "/notify mention", "/notify trigger" };
    for (int i = 0; i < ARRAY_SIZE(boolean_choices2); i++) {
        result = autocomplete_param_with_func(input, boolean_choices2[i], prefs_autocomplete_boolean_choice, previous, NULL);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/notify", notify_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_autoaway_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/autoaway mode", autoaway_mode_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/autoaway time", autoaway_presence_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/autoaway message", autoaway_presence_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/autoaway check", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/autoaway", autoaway_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_log_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/log rotate", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_func(input, "/log shared", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/log level", log_level_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/log", log_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_autoconnect_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/autoconnect set", accounts_find_enabled, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/autoconnect", autoconnect_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

#ifdef HAVE_LIBOTR
static char*
_otr_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {
        found = autocomplete_param_with_func(input, "/otr start", roster_contact_autocomplete, previous, NULL);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/otr log", otr_log_ac, TRUE, previous);
    if (found) {
        return found;
    }

    // /otr policy always user@server.com
    if (conn_status == JABBER_CONNECTED) {
        gboolean result;
        auto_gcharv gchar** args = parse_args(input, 2, 3, &result);
        if (result && (strcmp(args[0], "policy") == 0)) {
            GString* beginning = g_string_new("/otr ");
            g_string_append(beginning, args[0]);
            g_string_append(beginning, " ");
            if (args[1]) {
                g_string_append(beginning, args[1]);
            }

            found = autocomplete_param_with_func(input, beginning->str, roster_contact_autocomplete, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
    }

    found = autocomplete_param_with_ac(input, "/otr policy", otr_policy_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/otr sendfile", otr_sendfile_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/otr", otr_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}
#endif

#ifdef HAVE_LIBGPGME
static char*
_pgp_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {
        found = autocomplete_param_with_func(input, "/pgp start", roster_contact_autocomplete, previous, NULL);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/pgp log", pgp_log_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/pgp sendfile", pgp_sendfile_ac, TRUE, previous);
    if (found) {
        return found;
    }

    gboolean result;
    auto_gcharv gchar** args = parse_args(input, 2, 3, &result);
    if ((strncmp(input, "/pgp", 4) == 0) && (result == TRUE)) {
        GString* beginning = g_string_new("/pgp ");
        g_string_append(beginning, args[0]);
        if (args[1]) {
            g_string_append(beginning, " ");
            g_string_append(beginning, args[1]);
        }
        found = autocomplete_param_with_func(input, beginning->str, p_gpg_autocomplete_key, previous, NULL);
        g_string_free(beginning, TRUE);
        if (found) {
            return found;
        }
    }

    if (conn_status == JABBER_CONNECTED) {
        found = autocomplete_param_with_func(input, "/pgp setkey", roster_barejid_autocomplete, previous, NULL);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/pgp", pgp_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_ox_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_ac(input, "/ox", ox_ac, TRUE, previous);
    if (found) {
        return found;
    }

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {
        found = autocomplete_param_with_func(input, "/ox start", roster_contact_autocomplete, previous, NULL);
        if (found) {
            return found;
        }

        found = autocomplete_param_with_func(input, "/ox discover", roster_barejid_autocomplete, previous, NULL);
        if (found) {
            return found;
        }

        found = autocomplete_param_with_func(input, "/ox setkey", roster_barejid_autocomplete, previous, NULL);
        if (found) {
            return found;
        }

        found = autocomplete_param_with_func(input, "/ox request", roster_barejid_autocomplete, previous, NULL);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/ox log", ox_log_ac, TRUE, previous);
    if (found) {
        return found;
    }

    if (strncmp(input, "/ox announce ", 13) == 0) {
        return cmd_ac_complete_filepath(input, "/ox announce", previous);
    }

    return NULL;
}
#endif

#ifdef HAVE_OMEMO
static char*
_omemo_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_ac(input, "/omemo log", omemo_log_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/omemo policy", omemo_policy_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/omemo trustmode", omemo_trustmode_ac, TRUE, previous);
    if (found) {
        return found;
    }

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {
        found = autocomplete_param_with_func(input, "/omemo start", roster_contact_autocomplete, previous, NULL);
        if (found) {
            return found;
        }

        found = autocomplete_param_with_func(input, "/omemo fingerprint", roster_contact_autocomplete, previous, NULL);
        if (found) {
            return found;
        }

        if (window->type == WIN_CHAT) {
            ProfChatWin* chatwin = (ProfChatWin*)window;
            assert(chatwin->memcheck == PROFCHATWIN_MEMCHECK);
            found = autocomplete_param_with_func(input, "/omemo trust", omemo_fingerprint_autocomplete, previous, chatwin->barejid);
            if (found) {
                return found;
            }
        } else {
            found = autocomplete_param_with_func(input, "/omemo trust", roster_contact_autocomplete, previous, NULL);
            if (found) {
                return found;
            }

            int num_tokens = count_tokens(input);
            if (num_tokens == 4) {
                gboolean result;
                auto_gcharv gchar** args = parse_args(input, 2, 3, &result);
                if (result) {
                    auto_gchar gchar* jid = g_strdup(args[1]);
                    found = autocomplete_param_no_with_func(input, "/omemo trust", 4, omemo_fingerprint_autocomplete, previous, jid);
                    if (found) {
                        return found;
                    }
                }
            }
        }

        if (window->type == WIN_CHAT) {
            ProfChatWin* chatwin = (ProfChatWin*)window;
            assert(chatwin->memcheck == PROFCHATWIN_MEMCHECK);
            found = autocomplete_param_with_func(input, "/omemo untrust", omemo_fingerprint_autocomplete, previous, chatwin->barejid);
            if (found) {
                return found;
            }
        } else {
            found = autocomplete_param_with_func(input, "/omemo untrust", roster_contact_autocomplete, previous, NULL);
            if (found) {
                return found;
            }

            int num_tokens = count_tokens(input);
            if (num_tokens == 4) {
                gboolean result;
                auto_gcharv gchar** args = parse_args(input, 2, 3, &result);
                if (result) {
                    auto_gchar gchar* jid = g_strdup(args[1]);
                    found = autocomplete_param_no_with_func(input, "/omemo untrust", 4, omemo_fingerprint_autocomplete, previous, jid);
                    if (found) {
                        return found;
                    }
                }
            }
        }
    }

    found = autocomplete_param_with_ac(input, "/omemo", omemo_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}
#endif

static char*
_plugins_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (strncmp(input, "/plugins install ", 17) == 0) {
        return cmd_ac_complete_filepath(input, "/plugins install", previous);
    }

    if (strncmp(input, "/plugins update ", 16) == 0) {
        return cmd_ac_complete_filepath(input, "/plugins update", previous);
    }

    if (strncmp(input, "/plugins load ", 14) == 0) {
        if (plugins_load_ac == NULL) {
            plugins_load_ac = autocomplete_new();
            GSList* plugins = plugins_unloaded_list();
            GSList* curr = plugins;
            while (curr) {
                autocomplete_add(plugins_load_ac, curr->data);
                curr = g_slist_next(curr);
            }
            g_slist_free_full(plugins, g_free);
        }
        result = autocomplete_param_with_ac(input, "/plugins load", plugins_load_ac, TRUE, previous);
        if (result) {
            return result;
        }
    }

    if (strncmp(input, "/plugins reload ", 16) == 0) {
        if (plugins_reload_ac == NULL) {
            plugins_reload_ac = autocomplete_new();
            GList* plugins = plugins_loaded_list();
            GList* curr = plugins;
            while (curr) {
                autocomplete_add(plugins_reload_ac, curr->data);
                curr = g_list_next(curr);
            }
            g_list_free(plugins);
        }
        result = autocomplete_param_with_ac(input, "/plugins reload", plugins_reload_ac, TRUE, previous);
        if (result) {
            return result;
        }
    }

    if (strncmp(input, "/plugins unload ", 16) == 0) {
        if (plugins_unload_ac == NULL) {
            plugins_unload_ac = autocomplete_new();
            GList* plugins = plugins_loaded_list();
            GList* curr = plugins;
            while (curr) {
                autocomplete_add(plugins_unload_ac, curr->data);
                curr = g_list_next(curr);
            }
            g_list_free(plugins);
        }
        result = autocomplete_param_with_ac(input, "/plugins unload", plugins_unload_ac, TRUE, previous);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/plugins", plugins_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_theme_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (strncmp(input, "/theme load ", 12) == 0) {
        if (theme_load_ac == NULL) {
            theme_load_ac = autocomplete_new();
            GSList* themes = theme_list();
            GSList* curr = themes;

            while (curr) {
                autocomplete_add(theme_load_ac, curr->data);
                curr = g_slist_next(curr);
            }

            g_slist_free_full(themes, g_free);
            autocomplete_add(theme_load_ac, "default");
        }

        result = autocomplete_param_with_ac(input, "/theme load", theme_load_ac, TRUE, previous);
        if (result) {
            return result;
        }
    }

    if (strncmp(input, "/theme full-load ", 17) == 0) {
        if (theme_load_ac == NULL) {
            theme_load_ac = autocomplete_new();
            GSList* themes = theme_list();
            GSList* curr = themes;

            while (curr) {
                autocomplete_add(theme_load_ac, curr->data);
                curr = g_slist_next(curr);
            }

            g_slist_free_full(themes, g_free);
            autocomplete_add(theme_load_ac, "default");
        }

        result = autocomplete_param_with_ac(input, "/theme full-load", theme_load_ac, TRUE, previous);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/theme", theme_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_script_autocomplete_func(const char* const prefix, gboolean previous, void* context)
{
    if (script_show_ac == NULL) {
        script_show_ac = autocomplete_new();
        GSList* scripts = scripts_list();
        GSList* curr = scripts;
        while (curr) {
            autocomplete_add(script_show_ac, curr->data);
            curr = g_slist_next(curr);
        }
        g_slist_free_full(scripts, g_free);
    }

    return autocomplete_complete(script_show_ac, prefix, FALSE, previous);
}

static char*
_script_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;
    if (strncmp(input, "/script show ", 13) == 0) {
        result = autocomplete_param_with_func(input, "/script show", _script_autocomplete_func, previous, NULL);
        if (result) {
            return result;
        }
    }

    if (strncmp(input, "/script run ", 12) == 0) {
        result = autocomplete_param_with_func(input, "/script run", _script_autocomplete_func, previous, NULL);
        if (result) {
            return result;
        }
    }

    result = autocomplete_param_with_ac(input, "/script", script_ac, TRUE, previous);
    if (result) {
        return result;
    }

    return NULL;
}

static char*
_resource_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    jabber_conn_status_t conn_status = connection_get_status();
    if (conn_status == JABBER_CONNECTED && window->type == WIN_CHAT) {
        ProfChatWin* chatwin = (ProfChatWin*)window;
        assert(chatwin->memcheck == PROFCHATWIN_MEMCHECK);
        PContact contact = roster_get_contact(chatwin->barejid);
        if (contact) {
            Autocomplete ac = p_contact_resource_ac(contact);
            found = autocomplete_param_with_ac(input, "/resource set", ac, FALSE, previous);
            if (found) {
                return found;
            }
        }
    }

    found = autocomplete_param_with_func(input, "/resource title", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_func(input, "/resource message", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/resource", resource_ac, FALSE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_wintitle_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_func(input, "/wintitle show", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_func(input, "/wintitle goodbye", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/wintitle", wintitle_ac, FALSE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_inpblock_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_func(input, "/inpblock dynamic", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/inpblock", inpblock_ac, FALSE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_form_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    if (window->type != WIN_CONFIG) {
        return NULL;
    }

    char* found = NULL;

    ProfConfWin* confwin = (ProfConfWin*)window;
    DataForm* form = confwin->form;
    if (form) {
        found = autocomplete_param_with_ac(input, "/form help", form->tag_ac, TRUE, previous);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/form", form_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_form_field_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    if (window->type != WIN_CONFIG) {
        return NULL;
    }

    char* found = NULL;

    ProfConfWin* confwin = (ProfConfWin*)window;
    DataForm* form = confwin->form;
    if (form == NULL) {
        return NULL;
    }

    gchar** split = g_strsplit(input, " ", 0);

    if (g_strv_length(split) == 3) {
        char* field_tag = split[0] + 1;
        if (form_tag_exists(form, field_tag)) {
            form_field_type_t field_type = form_get_field_type(form, field_tag);
            Autocomplete value_ac = form_get_value_ac(form, field_tag);
            ;
            GString* beginning = g_string_new(split[0]);
            g_string_append(beginning, " ");
            g_string_append(beginning, split[1]);

            if (((g_strcmp0(split[1], "add") == 0) || (g_strcmp0(split[1], "remove") == 0))
                && field_type == FIELD_LIST_MULTI) {
                found = autocomplete_param_with_ac(input, beginning->str, value_ac, TRUE, previous);

            } else if ((g_strcmp0(split[1], "remove") == 0) && field_type == FIELD_TEXT_MULTI) {
                found = autocomplete_param_with_ac(input, beginning->str, value_ac, TRUE, previous);

            } else if ((g_strcmp0(split[1], "remove") == 0) && field_type == FIELD_JID_MULTI) {
                found = autocomplete_param_with_ac(input, beginning->str, value_ac, TRUE, previous);
            }

            g_string_free(beginning, TRUE);
        }

    } else if (g_strv_length(split) == 2) {
        char* field_tag = split[0] + 1;
        if (form_tag_exists(form, field_tag)) {
            form_field_type_t field_type = form_get_field_type(form, field_tag);
            Autocomplete value_ac = form_get_value_ac(form, field_tag);
            ;

            switch (field_type) {
            case FIELD_BOOLEAN:
                found = autocomplete_param_with_func(input, split[0], prefs_autocomplete_boolean_choice, previous, NULL);
                break;
            case FIELD_LIST_SINGLE:
                found = autocomplete_param_with_ac(input, split[0], value_ac, TRUE, previous);
                break;
            case FIELD_LIST_MULTI:
            case FIELD_JID_MULTI:
            case FIELD_TEXT_MULTI:
                found = autocomplete_param_with_ac(input, split[0], form_field_multi_ac, TRUE, previous);
                break;
            default:
                break;
            }
        }
    }

    g_strfreev(split);

    return found;
}

static char*
_occupants_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_ac(input, "/occupants default show", occupants_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants char", occupants_char_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_func(input, "/occupants color", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants default hide", occupants_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants default", occupants_default_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants show", occupants_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants hide", occupants_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants header char", roster_char_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants header", occupants_header_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_func(input, "/occupants wrap", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/occupants", occupants_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_time_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_ac(input, "/time statusbar", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time lastactivity", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time console", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time chat", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time muc", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time config", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time private", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time xml", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time all", time_format_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/time", time_ac, TRUE, previous);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_kick_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    if (window->type != WIN_MUC) {
        return NULL;
    }

    ProfMucWin* mucwin = (ProfMucWin*)window;
    assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
    Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);
    if (nick_ac == NULL) {
        return NULL;
    }

    char* result = autocomplete_param_with_ac(input, "/kick", nick_ac, TRUE, previous);

    return result;
}

static char*
_ban_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    if (window->type != WIN_MUC) {
        return NULL;
    }

    ProfMucWin* mucwin = (ProfMucWin*)window;
    assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
    Autocomplete jid_ac = muc_roster_jid_ac(mucwin->roomjid);
    if (jid_ac == NULL) {
        return NULL;
    }

    char* result = autocomplete_param_with_ac(input, "/ban", jid_ac, TRUE, previous);

    return result;
}

static char*
_affiliation_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (window->type == WIN_MUC) {
        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
        gboolean parse_result;
        Autocomplete jid_ac = muc_roster_jid_ac(mucwin->roomjid);

        auto_gcharv gchar** args = parse_args(input, 2, 3, &parse_result);

        if ((strncmp(input, "/affiliation", 12) == 0) && (parse_result == TRUE)) {
            GString* beginning = g_string_new("/affiliation ");
            g_string_append(beginning, args[0]);
            g_string_append(beginning, " ");
            if (args[1]) {
                g_string_append(beginning, args[1]);
            }

            result = autocomplete_param_with_ac(input, beginning->str, jid_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (result) {
                return result;
            }
        }
    }

    result = autocomplete_param_with_ac(input, "/affiliation set", affiliation_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/affiliation list", affiliation_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/affiliation", affiliation_cmd_ac, TRUE, previous);

    return result;
}

static char*
_role_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (window->type == WIN_MUC) {
        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
        gboolean parse_result;
        Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);

        auto_gcharv gchar** args = parse_args(input, 2, 3, &parse_result);

        if ((strncmp(input, "/role", 5) == 0) && (parse_result == TRUE)) {
            GString* beginning = g_string_new("/role ");
            g_string_append(beginning, args[0]);
            g_string_append(beginning, " ");
            if (args[1]) {
                g_string_append(beginning, args[1]);
            }

            result = autocomplete_param_with_ac(input, beginning->str, nick_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (result) {
                return result;
            }
        }
    }

    result = autocomplete_param_with_ac(input, "/role set", role_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/role list", role_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/role", role_cmd_ac, TRUE, previous);

    return result;
}

static char*
_wins_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/wins", wins_ac, TRUE, previous);

    return result;
}

static char*
_tls_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/tls revoke", tlscerts_complete, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/tls cert", tlscerts_complete, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/tls certpath", tls_certpath_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/tls", tls_ac, TRUE, previous);

    return result;
}

static char*
_titlebar_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/titlebar show", titlebar_show_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/titlebar hide", titlebar_show_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/titlebar", titlebar_ac, TRUE, previous);

    return result;
}

static char*
_receipts_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/receipts send", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/receipts request", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/receipts", receipts_ac, TRUE, previous);

    return result;
}

static char*
_alias_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/alias remove", aliases_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/alias", alias_ac, TRUE, previous);

    return result;
}

static char*
_connect_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;
    gboolean result = FALSE;

    auto_gcharv gchar** args = parse_args(input, 1, 9, &result);

    if (result) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        if ((num_args == 1 && space_at_end) || (num_args == 2 && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s", args[0]);
            found = autocomplete_param_with_ac(input, beginning->str, connect_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 2 && space_at_end && (g_strcmp0(args[1], "tls") == 0))
            || (num_args == 3 && (g_strcmp0(args[1], "tls") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            found = autocomplete_param_with_ac(input, beginning->str, tls_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end) || (num_args == 4 && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, connect_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 4 && space_at_end && (g_strcmp0(args[3], "tls") == 0))
            || (num_args == 5 && (g_strcmp0(args[3], "tls") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s", args[0], args[1], args[2], args[3]);
            found = autocomplete_param_with_ac(input, beginning->str, tls_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 5 && space_at_end) || (num_args == 6 && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4]);
            found = autocomplete_param_with_ac(input, beginning->str, connect_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 6 && space_at_end && (g_strcmp0(args[5], "tls") == 0))
            || (num_args == 7 && (g_strcmp0(args[5], "tls") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5]);
            found = autocomplete_param_with_ac(input, beginning->str, tls_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 7 && space_at_end) || (num_args == 8 && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
            found = autocomplete_param_with_ac(input, beginning->str, connect_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 8 && space_at_end && (g_strcmp0(args[7], "tls") == 0))
            || (num_args == 9 && (g_strcmp0(args[7], "tls") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            found = autocomplete_param_with_ac(input, beginning->str, tls_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }

        /* auth option */

        if ((num_args == 2 && space_at_end && (g_strcmp0(args[1], "auth") == 0))
            || (num_args == 3 && (g_strcmp0(args[1], "auth") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            found = autocomplete_param_with_ac(input, beginning->str, auth_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 4 && space_at_end && (g_strcmp0(args[3], "auth") == 0))
            || (num_args == 5 && (g_strcmp0(args[3], "auth") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s", args[0], args[1], args[2], args[3]);
            found = autocomplete_param_with_ac(input, beginning->str, auth_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 6 && space_at_end && (g_strcmp0(args[5], "auth") == 0))
            || (num_args == 7 && (g_strcmp0(args[5], "auth") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5]);
            found = autocomplete_param_with_ac(input, beginning->str, auth_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 8 && space_at_end && (g_strcmp0(args[7], "auth") == 0))
            || (num_args == 9 && (g_strcmp0(args[7], "auth") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/connect");
            g_string_append_printf(beginning, " %s %s %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            found = autocomplete_param_with_ac(input, beginning->str, auth_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
    }

    found = autocomplete_param_with_func(input, "/connect", accounts_find_enabled, previous, NULL);
    if (found) {
        return found;
    }

    return NULL;
}

static char*
_help_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/help commands", help_commands_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/help", help_ac, TRUE, previous);
    return result;
}

static char*
_join_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;
    gboolean result = FALSE;

    auto_gcharv gchar** args = parse_args(input, 1, 5, &result);

    if (result) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        if ((num_args == 1 && space_at_end) || (num_args == 2 && !space_at_end)) {
            GString* beginning = g_string_new("/join");
            g_string_append_printf(beginning, " %s", args[0]);
            found = autocomplete_param_with_ac(input, beginning->str, join_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end) || (num_args == 4 && !space_at_end)) {
            GString* beginning = g_string_new("/join");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, join_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
    }

    found = autocomplete_param_with_func(input, "/join", bookmark_find, previous, NULL);

    return found;
}

static char*
_console_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/console chat", console_msg_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/console muc", console_msg_ac, TRUE, previous);
    if (result) {
        return result;
    }
    result = autocomplete_param_with_ac(input, "/console private", console_msg_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/console", console_ac, TRUE, previous);
    return result;
}

static char*
_win_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = autocomplete_param_with_func(input, "/win", win_autocomplete, previous, NULL);

    if (result) {
        return result;
    }

    char* unquoted = strip_arg_quotes(input);
    result = autocomplete_param_with_func(unquoted, "/win", roster_contact_autocomplete, previous, NULL);
    free(unquoted);
    return result;
}

static char*
_close_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    return autocomplete_param_with_func(input, "/close", win_close_autocomplete, previous, NULL);
}

static char*
_sendfile_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    return cmd_ac_complete_filepath(input, "/sendfile", previous);
}

static char*
_subject_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (window->type == WIN_MUC) {
        if ((g_strcmp0(input, "/subject edit ") == 0)
            || (g_strcmp0(input, "/subject edit \"") == 0)) {
            ProfMucWin* mucwin = (ProfMucWin*)window;
            assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);

            char* subject = muc_subject(mucwin->roomjid);
            if (subject) {
                GString* result_str = g_string_new("/subject edit \"");
                g_string_append(result_str, subject);
                g_string_append(result_str, "\"");

                result = result_str->str;
                g_string_free(result_str, FALSE);
            }
        }
    }
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/subject", subject_ac, TRUE, previous);

    return result;
}

static char*
_account_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;
    gboolean result = FALSE;

    auto_gcharv gchar** args = parse_args(input, 2, 4, &result);
    if (result && (strcmp(args[0], "set") == 0)) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        if ((num_args == 2 && space_at_end) || (num_args == 3 && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            found = autocomplete_param_with_ac(input, beginning->str, account_set_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
#ifdef HAVE_LIBOTR
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "otr") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "otr") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, otr_policy_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                g_strfreev(args);
                return found;
            }
        }
#endif
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "status") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "status") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, account_status_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "tls") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "tls") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, tls_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "auth") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "auth") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_ac(input, beginning->str, auth_property_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "startscript") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "startscript") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_func(input, beginning->str, _script_autocomplete_func, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "theme") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "theme") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            if (theme_load_ac == NULL) {
                theme_load_ac = autocomplete_new();
                GSList* themes = theme_list();
                GSList* curr = themes;
                while (curr) {
                    autocomplete_add(theme_load_ac, curr->data);
                    curr = g_slist_next(curr);
                }
                g_slist_free_full(themes, g_free);
                autocomplete_add(theme_load_ac, "default");
            }
            found = autocomplete_param_with_ac(input, beginning->str, theme_load_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
#ifdef HAVE_LIBGPGME
        if ((num_args == 3 && space_at_end && (g_strcmp0(args[2], "pgpkeyid") == 0))
            || (num_args == 4 && (g_strcmp0(args[2], "pgpkeyid") == 0) && !space_at_end)) {
            GString* beginning = g_string_new("/account");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_func(input, beginning->str, p_gpg_autocomplete_key, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
#endif
    }

    if ((strncmp(input, "/account clear", 14) == 0) && (result == TRUE)) {
        GString* beginning = g_string_new("/account clear ");
        g_string_append(beginning, args[1]);
        found = autocomplete_param_with_ac(input, beginning->str, account_clear_ac, TRUE, previous);
        g_string_free(beginning, TRUE);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/account default", account_default_ac, TRUE, previous);
    if (found) {
        return found;
    }

    gchar* account_choice[] = { "/account set", "/account show", "/account enable",
                                "/account disable", "/account rename", "/account clear", "/account remove",
                                "/account default set" };

    for (int i = 0; i < ARRAY_SIZE(account_choice); i++) {
        found = autocomplete_param_with_func(input, account_choice[i], accounts_find_all, previous, NULL);
        if (found) {
            return found;
        }
    }

    found = autocomplete_param_with_ac(input, "/account", account_ac, TRUE, previous);
    return found;
}

static char*
_presence_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_func(input, "/presence titlebar", prefs_autocomplete_boolean_choice, previous, NULL);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/presence console", presence_setting_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/presence chat", presence_setting_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/presence room", presence_setting_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/presence", presence_ac, TRUE, previous);

    return found;
}

static char*
_rooms_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;
    gboolean result = FALSE;

    auto_gcharv gchar** args = parse_args(input, 0, 4, &result);

    if (result) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        if (num_args <= 1) {
            found = autocomplete_param_with_ac(input, "/rooms", rooms_all_ac, TRUE, previous);
            if (found) {
                return found;
            }
        }
        if ((num_args == 1 && g_strcmp0(args[0], "service") == 0 && space_at_end) || (num_args == 2 && g_strcmp0(args[0], "service") == 0 && !space_at_end)) {
            found = autocomplete_param_with_func(input, "/rooms service", muc_confserver_find, previous, NULL);
            if (found) {
                return found;
            }
        }
        if ((num_args == 1 && g_strcmp0(args[0], "cache") == 0 && space_at_end) || (num_args == 2 && g_strcmp0(args[0], "cache") == 0 && !space_at_end)) {
            found = autocomplete_param_with_ac(input, "/rooms cache", rooms_cache_ac, TRUE, previous);
            if (found) {
                return found;
            }
        }
        if ((num_args == 2 && space_at_end) || (num_args == 3 && !space_at_end)) {
            GString* beginning = g_string_new("/rooms");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            found = autocomplete_param_with_ac(input, beginning->str, rooms_list_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args == 3 && g_strcmp0(args[2], "service") == 0 && space_at_end) || (num_args == 4 && g_strcmp0(args[2], "service") == 0 && !space_at_end)) {
            GString* beginning = g_string_new("/rooms");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            found = autocomplete_param_with_func(input, beginning->str, muc_confserver_find, previous, NULL);
            g_string_free(beginning, TRUE);
            if (found) {
                return found;
            }
        }
        if ((num_args >= 2) && g_strcmp0(args[0], "cache") == 0) {
            return NULL;
        }
    }

    return NULL;
}

static char*
_statusbar_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* found = NULL;

    found = autocomplete_param_with_ac(input, "/statusbar", statusbar_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/statusbar show", statusbar_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/statusbar hide", statusbar_show_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/statusbar self", statusbar_self_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/statusbar chat", statusbar_chat_ac, TRUE, previous);
    if (found) {
        return found;
    }

    found = autocomplete_param_with_ac(input, "/statusbar room", statusbar_room_ac, TRUE, previous);

    return found;
}

static char*
_clear_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/clear", clear_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/clear persist_history", prefs_autocomplete_boolean_choice, previous, NULL);
    return result;
}

static char*
_invite_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/invite", invite_ac, TRUE, previous);
    if (result) {
        return result;
    }

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {
        result = autocomplete_param_with_func(input, "/invite send", roster_contact_autocomplete, previous, NULL);
        if (result) {
            return result;
        }

        result = autocomplete_param_with_func(input, "/invite decline", muc_invites_find, previous, NULL);
        if (result) {
            return result;
        }
    }

    return NULL;
}

static char*
_status_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/status", status_ac, TRUE, previous);
    if (result) {
        return result;
    }

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {

        // complete with: online, away etc.
        result = autocomplete_param_with_ac(input, "/status set", account_status_ac, TRUE, previous);
        if (result) {
            return result;
        }

        // Remove quote character before and after names when doing autocomplete
        char* unquoted = strip_arg_quotes(input);

        // MUC completion with nicknames
        if (window->type == WIN_MUC) {
            ProfMucWin* mucwin = (ProfMucWin*)window;
            assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
            Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);
            if (nick_ac) {
                result = autocomplete_param_with_ac(unquoted, "/status get", nick_ac, TRUE, previous);
                if (result) {
                    free(unquoted);
                    return result;
                }
            }
            // roster completion
        } else {
            result = autocomplete_param_with_func(unquoted, "/status get", roster_contact_autocomplete, previous, NULL);
            if (result) {
                free(unquoted);
                return result;
            }
        }

        free(unquoted);
    }

    return NULL;
}

static char*
_logging_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/logging", logging_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/logging chat", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/logging group", logging_group_ac, TRUE, previous);
    return result;
}

static char*
_color_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_func(input, "/color own", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/color", color_ac, TRUE, previous);
    return result;
}

static char*
_avatar_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/avatar", avatar_ac, TRUE, previous);
    if (result) {
        return result;
    }

    jabber_conn_status_t conn_status = connection_get_status();
    if (conn_status == JABBER_CONNECTED) {
        result = cmd_ac_complete_filepath(input, "/avatar set", previous);
        if (result) {
            return result;
        }

        result = autocomplete_param_with_func(input, "/avatar get", roster_barejid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }

        result = autocomplete_param_with_func(input, "/avatar open", roster_barejid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
    }

    return NULL;
}

static char*
_correction_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/correction", correction_ac, TRUE, previous);

    return result;
}

static char*
_correct_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    GString* result_str = g_string_new("/correct ");
    g_string_append(result_str, win_get_last_sent_message(window));
    char* result = result_str->str;
    g_string_free(result_str, FALSE);

    return result;
}

static char*
_software_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    if (window->type == WIN_CHAT) {
        ProfChatWin* chatwin = (ProfChatWin*)window;
        assert(chatwin->memcheck == PROFCHATWIN_MEMCHECK);

        GString* search_str = g_string_new("/software ");
        g_string_append(search_str, chatwin->barejid);
        result = autocomplete_param_with_func(search_str->str, "/software", roster_fulljid_autocomplete, previous, NULL);
        g_string_free(search_str, TRUE);
    } else if (window->type == WIN_MUC) {
        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);

        Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);

        if (nick_ac) {
            result = autocomplete_param_with_ac(input, "/software", nick_ac, TRUE, previous);
            if (result) {
                return result;
            }
        }
    } else {
        result = autocomplete_param_with_func(input, "/software", roster_fulljid_autocomplete, previous, NULL);
        if (result) {
            return result;
        }
    }

    return result;
}

static char*
_url_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/url", url_ac, TRUE, previous);
    if (result) {
        return result;
    }

    if (window->type == WIN_CHAT || window->type == WIN_MUC || window->type == WIN_PRIVATE) {
        result = autocomplete_param_with_func(input, "/url open", wins_get_url, previous, window);
        if (result) {
            return result;
        }

        gboolean arg_result;
        auto_gcharv gchar** args = parse_args(input, 1, 8, &arg_result);
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);

        if (arg_result) {
            if ((num_args == 1 && space_at_end) || (num_args == 2 && !space_at_end)) {
                result = autocomplete_param_with_func(input, "/url save", wins_get_url, previous, window);
            } else if ((num_args == 2 && space_at_end) || (num_args == 3 && !space_at_end)) {
                gchar* cmd = g_strdup_printf("/url save %s", args[1]);
                result = cmd_ac_complete_filepath(input, cmd, previous);
                g_free(cmd);
            }
        }
    }

    return result;
}

static char*
_executable_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/executable", executable_ac, TRUE, previous);

    return result;
}

static char*
_lastactivity_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/lastactivity", lastactivity_ac, TRUE, previous);
    if (result) {
        return result;
    }

    jabber_conn_status_t conn_status = connection_get_status();

    if (conn_status == JABBER_CONNECTED) {

        result = autocomplete_param_with_func(input, "/lastactivity set", prefs_autocomplete_boolean_choice, previous, NULL);
        if (result) {
            return result;
        }
        result = autocomplete_param_with_func(input, "/lastactivity get", roster_barejid_autocomplete, previous, NULL);
    }

    return result;
}

static char*
_intype_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;
    result = autocomplete_param_with_func(input, "/intype console", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_func(input, "/intype titlebar", prefs_autocomplete_boolean_choice, previous, NULL);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/intype", intype_ac, FALSE, previous);
    return result;
}

static char*
_mood_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/mood", mood_ac, TRUE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/mood set", mood_type_ac, FALSE, previous);
    if (result) {
        return result;
    }

    return result;
}

static char*
_strophe_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/strophe sm", strophe_sm_ac, FALSE, previous);
    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/strophe verbosity", strophe_verbosity_ac, FALSE, previous);
    if (result) {
        return result;
    }

    return autocomplete_param_with_ac(input, "/strophe", strophe_ac, FALSE, previous);
}

static char*
_adhoc_cmd_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    result = autocomplete_param_with_ac(input, "/cmd", adhoc_cmd_ac, TRUE, previous);

    return result;
}

static char*
_vcard_autocomplete(ProfWin* window, const char* const input, gboolean previous)
{
    char* result = NULL;

    gboolean parse_result = FALSE;
    auto_gcharv gchar** args = parse_args(input, 0, 7, &parse_result);

    if (parse_result && (g_strcmp0(args[0], "set") == 0)) {
        gboolean space_at_end = g_str_has_suffix(input, " ");
        int num_args = g_strv_length(args);
        gboolean is_num = TRUE;

        if (num_args >= 2) {
            for (int i = 0; i < strlen(args[1]); i++) {
                if (!isdigit((int)args[1][i])) {
                    is_num = FALSE;
                    break;
                }
            }
        }

        if ((num_args == 2 && space_at_end && is_num) || (num_args == 3 && !space_at_end && is_num)) {
            GString* beginning = g_string_new("/vcard");
            g_string_append_printf(beginning, " %s %s", args[0], args[1]);
            result = autocomplete_param_with_ac(input, beginning->str, vcard_set_param_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (result) {
                return result;
            }
        } else if ((num_args == 3 && space_at_end && is_num && (g_strcmp0(args[2], "type") == 0)) || (num_args == 4 && !space_at_end && is_num && (g_strcmp0(args[2], "type") == 0))) {
            GString* beginning = g_string_new("/vcard");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            result = autocomplete_param_with_ac(input, beginning->str, vcard_address_type_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (result) {
                return result;
            }
        } else if ((num_args == 3 && space_at_end && is_num && autocomplete_contains(vcard_togglable_param_ac, args[2])) || (num_args == 4 && !space_at_end && is_num && autocomplete_contains(vcard_togglable_param_ac, args[2]))) {
            GString* beginning = g_string_new("/vcard");
            g_string_append_printf(beginning, " %s %s %s", args[0], args[1], args[2]);
            result = autocomplete_param_with_ac(input, beginning->str, vcard_toggle_ac, TRUE, previous);
            g_string_free(beginning, TRUE);
            if (result) {
                return result;
            }
        } else {
            result = autocomplete_param_with_ac(input, "/vcard set name", vcard_name_ac, TRUE, previous);

            if (result) {
                return result;
            }

            result = autocomplete_param_with_ac(input, "/vcard set", vcard_set_ac, TRUE, previous);

            if (result) {
                return result;
            }
        }
    }

    result = autocomplete_param_with_ac(input, "/vcard add", vcard_element_ac, TRUE, previous);

    if (result) {
        return result;
    }

    if (window->type == WIN_MUC) {
        char* unquoted = strip_arg_quotes(input);

        ProfMucWin* mucwin = (ProfMucWin*)window;
        assert(mucwin->memcheck == PROFMUCWIN_MEMCHECK);
        Autocomplete nick_ac = muc_roster_ac(mucwin->roomjid);

        if (nick_ac) {
            result = autocomplete_param_with_ac(unquoted, "/vcard get", nick_ac, TRUE, previous);
            if (result) {
                free(unquoted);
                return result;
            }

            result = autocomplete_param_with_ac(unquoted, "/vcard photo open", nick_ac, TRUE, previous);
            if (result) {
                free(unquoted);
                return result;
            }

            result = autocomplete_param_with_ac(unquoted, "/vcard photo save", nick_ac, TRUE, previous);
            if (result) {
                free(unquoted);
                return result;
            }
        }
        free(unquoted);
    } else {
        char* unquoted = strip_arg_quotes(input);

        result = autocomplete_param_with_func(unquoted, "/vcard get", roster_contact_autocomplete, previous, NULL);
        if (result) {
            free(unquoted);
            return result;
        }

        result = autocomplete_param_with_func(unquoted, "/vcard photo open", roster_contact_autocomplete, previous, NULL);
        if (result) {
            free(unquoted);
            return result;
        }

        result = autocomplete_param_with_func(unquoted, "/vcard photo save", roster_contact_autocomplete, previous, NULL);
        if (result) {
            free(unquoted);
            return result;
        }
        free(unquoted);
    }

    result = autocomplete_param_with_ac(input, "/vcard photo", vcard_photo_ac, TRUE, previous);

    if (result) {
        return result;
    }

    result = autocomplete_param_with_ac(input, "/vcard", vcard_ac, TRUE, previous);

    if (result) {
        return result;
    }

    return result;
}
