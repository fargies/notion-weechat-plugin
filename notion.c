/*
** Copyright (C) 2011 Fargier Sylvain <fargier.sylvain@free.fr>
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
**
** notion.c
**
**        Created on: May 31, 2011
**   Orignial Author: fargie_s
**
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "weechat-plugin.h"

WEECHAT_PLUGIN_NAME("notion");
WEECHAT_PLUGIN_DESCRIPTION("Notion notification extension");
WEECHAT_PLUGIN_AUTHOR("Fargier Sylvain <fargier.sylvain@free.fr>");
WEECHAT_PLUGIN_VERSION("0.1");
WEECHAT_PLUGIN_LICENSE("BSD");

struct t_weechat_plugin *weechat_plugin = NULL;

struct notion_plugin {
    int sock;
    struct sockaddr_un addr;
    struct t_hook *hotlist_hook;
} notion;

static uint16_t hotlist_count()
{
    struct t_infolist *info;

    info = weechat_infolist_get("hotlist", NULL, NULL);
    if (info) {
        uint16_t count = 0;

        while (weechat_infolist_next(info) == 1)
            ++count;

        weechat_infolist_free(info);

        return count;
    }
    else
        return 0;
}

static void notion_update(uint16_t count)
{
    char data[17];
    int len;
    int ret;

    if (notion.sock < 0)
        return;

    len = snprintf(data, 17, "irc: %u\n", count);

    if (len > 0)
    {
        ret = sendto(notion.sock, data, len, MSG_DONTWAIT,
                (const struct sockaddr *) &(notion.addr),
                sizeof (notion.addr));
        if (ret < 0)
            weechat_printf(NULL, "%snotion: %s(%i)", weechat_prefix("error"),
                    strerror(errno), errno);
    }

    if (count > 0)
        ret = sendto(notion.sock, "irc_hint: important\n", 20, MSG_DONTWAIT,
                (const struct sockaddr *) &(notion.addr),
                sizeof (notion.addr));
    else
        ret = sendto(notion.sock, "irc_hint: normal\n", 17, MSG_DONTWAIT,
                (const struct sockaddr *) &(notion.addr),
                sizeof (notion.addr));
    if (ret < 0)
        weechat_printf(NULL, "%snotion: %s(%i)", weechat_prefix("error"),
                strerror(errno), errno);
}

static int hotlist_changed_cb(void *data,
                              const char *signal,
                              const char *type_data,
                              void *signal_data)
{
    notion_update(hotlist_count());
    return WEECHAT_RC_OK;
}

int
weechat_plugin_init(struct t_weechat_plugin *plugin,
                    int argc, char *argv[])
{
    weechat_plugin = plugin;
    notion.sock = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (notion.sock < 0)
        return WEECHAT_RC_ERROR;

    memset(&(notion.addr), 0, sizeof(notion.addr));
    notion.addr.sun_family = AF_UNIX;
    strcpy(notion.addr.sun_path + 1, "notion_statusd");

    notion_update(hotlist_count());

    weechat_hook_signal("hotlist_changed",
                        hotlist_changed_cb,
                        NULL);

    return WEECHAT_RC_OK;
}

int
weechat_plugin_end (struct t_weechat_plugin *plugin)
{
    if (notion.sock != -1)
        close(notion.sock);

    weechat_unhook(notion.hotlist_hook);
    return WEECHAT_RC_OK;
}

