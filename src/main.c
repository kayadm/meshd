/*
 *
 * Meshd, Bluetooth mesh stack
 *
 * Copyright (C) 2017  Loic Poulain <loic.poulain@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <glib.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include "uuid.h"
#include "node.h"
#include "utils.h"
#include "workqueue.h"
#include "network.h"
#include "provision.h"
#include "transport.h"
#include "bearer.h"
#include "access.h"
#include "interfaces/interface.h"

GMainLoop *mainloop;
struct node_st node;

gboolean signal_handler_interrupt(gpointer data)
{
	g_main_loop_quit(mainloop);


	return G_SOURCE_CONTINUE;
}

static const struct option main_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "interactive", no_argument, NULL, 'i' },
	{ }
};
gboolean tmp_sendResponseMessage(gpointer d,uint16_t srcAddress,uint16_t targetAddress,uint16_t dstAddress,int rssi)
{
    char data[] = "01 ";

char datax[80];
    sprintf(datax,",%x,",targetAddress);
    char data1[88];
    sprintf(data1,"%d,",rssi);
    char data2[]=" ...";

    char *fusionData=malloc(strlen(data)+strlen(datax)+strlen(data1)+strlen(data2)+1);
    fusionData[0]='\0';
    strcat(fusionData,data);
    strcat(fusionData,datax);
    strcat(fusionData,data1);
    strcat(fusionData,data2);
/*	*/
    /*No need to control this part
        if (!net)
        g_message("No network");*/

    transport_up_send_access_msg(node.network_l->data,
                     fusionData, strlen(fusionData) , srcAddress, dstAddress, 0);

    free(fusionData);
    return true;
}


int main(int argc, char *argv[])
{

    bool interactive;
	mainloop = g_main_loop_new(NULL, FALSE);
	if (mainloop == NULL)
		return -ENOMEM;

	for (;;) {
		int opt = getopt_long(argc, argv, "hi", main_options, NULL);
		if (opt < 0)
			break;
			switch (opt) {
		case 'i':
			interactive = true;
			break;
		}
	}


	element_create(0);

	crypto_init();
	network_init();
    char uuid[16];
    UUID(uuid);
    memcpy(node.uuid,uuid,16);
	provision_init();
	bearer_adv_init();
	configuration_server_model_init();

	if (interactive)
		cmdline_init(STDIN_FILENO, STDOUT_FILENO);

	g_main_loop_run(mainloop);

	crypto_cleanup();
	network_cleanup();

	g_main_loop_unref(mainloop);

	return 0;
}
