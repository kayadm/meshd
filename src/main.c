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
#include <gtk/gtk.h>
#include <stdlib.h>
bool scanState=false;
GMainLoop *mainloop;
struct node_st node;
GtkEntry *nidentry;
GtkEntry *addrentry;
GtkEntry *dataentry;
GtkWidget *netLabel;
GtkWidget *resultLabel;
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
void updateLabel(char data[]){
    gtk_label_set_text(resultLabel,data);
}
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
void on_window_destroy ()
{
    mainloop=NULL;
    gtk_main_quit();
    exit(0);
}
void callProvision(){
    if (!scanState){
        gtk_label_set_text(resultLabel,"Scanning...");
        char *argument[1];
        argument[0]="on";
        cmd_scan_unprovisionned(1,argument);
        scanState=true;
    }else{
        gtk_label_set_text(resultLabel,"Scanning Stopped!!..");

        char *argument[1];
        argument[0]="off";
        cmd_scan_unprovisionned(1,argument);
        scanState=false;
    }
}
void callNetCreate(){
    gtk_label_set_text(resultLabel,"Network Created!!");
    cmd_create_network(0,0);
}

void callsendMesg(){
    gtk_label_set_text(resultLabel,"Sending Message!!");
    gchar *text;
    text = gtk_entry_get_text(GTK_ENTRY(nidentry));
    gchar *text1;
    text1 = gtk_entry_get_text(GTK_ENTRY(addrentry));
    gchar *text2;
    text2 = gtk_entry_get_text(GTK_ENTRY(dataentry));
    char *data[3];
    data[0]=text;
    data[1]=text1;
    data[2]=text2;
    tmp_sendmsg(3,data);
}

int main(int argc, char *argv[])
{
   GtkBuilder *builder;
    GtkWidget *window;
    GError *err = NULL;
    char *symlink = "./src/GLADE/meshinterface.glade";
    char actualpath [PATH_MAX + 1];
    char *ptr;

    ptr = realpath(symlink, actualpath);
    g_message(actualpath);

    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, actualpath, &err) == 0) {
        fprintf(stderr, "Error adding builder from file %s\n%s\n",
            actualpath, err->message);
    }
    gtk_builder_connect_signals(builder, NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "mywindow"));
    if (window == NULL) {
        fprintf(stderr, " File Object window not found\n");

    }
    g_signal_connect (window, "delete-event", G_CALLBACK (on_window_destroy), NULL);

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
 GdkColor color;
    GdkColor color2;
    GdkColor color3;
    GdkColor color4;

    gdk_color_parse ("#DCEAE6", &color);
    gdk_color_parse ("#DCEAE6", &color2);
    gdk_color_parse ("#DCEAE6", &color3);
    gdk_color_parse ("#DCEAE6", &color4);

 /*   GtkBox *box1=GTK_WIDGET(gtk_builder_get_object(builder, "allBoxes"));

  GtkBox *grid1=GTK_WIDGET(gtk_builder_get_object(builder, "gridUpper"));

    GtkBox *grid2=GTK_WIDGET(gtk_builder_get_object(builder, "gridMiddle"));
    GtkBox *grid3=GTK_WIDGET(gtk_builder_get_object(builder, "gridlower"));*/
nidentry=GTK_WIDGET(gtk_builder_get_object(builder, "netidtext"));
addrentry=GTK_WIDGET(gtk_builder_get_object(builder, "addrtext"));
dataentry=GTK_WIDGET(gtk_builder_get_object(builder, "datatext"));
resultLabel=GTK_WIDGET(gtk_builder_get_object(builder, "results"));

netLabel=GTK_WIDGET(gtk_builder_get_object(builder, "netListLabel"));

setLabel(resultLabel);
setNetListLabel(netLabel);
/*if (box1 == NULL) {
    fprintf(stderr, " Object window not found\n");

}*/
    //gtk_widget_modify_bg( GTK_WIDGET(box1), GTK_STATE_NORMAL, &color);
  /*  gtk_widget_modify_bg( GTK_WIDGET(grid1), GTK_STATE_NORMAL, &color2);

    gtk_widget_modify_bg( GTK_WIDGET(grid2), GTK_STATE_NORMAL, &color3);
    gtk_widget_modify_bg( GTK_WIDGET(grid3), GTK_STATE_NORMAL, &color4);*/


    element_create(0);
	crypto_init();
	network_init();
    char uuid[16];
    UUID(uuid);
    memcpy(node.uuid,uuid,16);
	provision_init();
	bearer_adv_init();
	configuration_server_model_init();


   /*asdasd if (interactive)
        cmdline_init(STDIN_FILENO, STDOUT_FILENO);*/

    g_object_unref (G_OBJECT (builder));
   // gtk_widget_show_all(box1);
    gtk_widget_show_all(window);
    gtk_main();

    g_main_loop_run(mainloop);

    crypto_cleanup();
    network_cleanup();

    g_main_loop_unref(mainloop);

	return 0;
}
