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

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include "access.h"
#include "node.h"
#include "utils.h"
#include "network.h"
int oldRSSI=-1000;
char *oldADDR;
static inline uint32_t access_msg_get_opcode(uint8_t *access_msg)
{
    uint32_t opcode = 0;

    /* An operation code (opcode) is an array of octets comprising 1, 2,
     * or 3 octets. The first octet of the opcode determines the number of
     * octets that are part of the opcode.
     */
    switch ((uint8_t)access_msg[0] & 0xC0) {
    case 0xC0: /* 3-octet opcode */
        opcode = access_msg[0] << 16;
        opcode += access_msg[1] << 8;
        opcode += access_msg[2];
        break;
    case 0x80: /* 2-octet opcode */
        opcode = access_msg[0] << 8;
        opcode += access_msg[1];
        break;
    default: /* 1-octet opcode */
        opcode = access_msg[0];
    }

    return opcode;
}

static inline int access_msg_set_opcode(uint8_t *access_msg, uint32_t opcode)
{

    if (opcode & 0x00FF0000) {
        access_msg[0] = opcode >> 16;
        access_msg[1] = opcode >> 8;
        access_msg[2] = opcode;
        return 3;
    } else if (opcode & 0x0000FF00) {
        access_msg[0] = opcode >> 8;
        access_msg[1] = opcode;
        return 2;
    } else {
        access_msg[0] = opcode;
        return 1;
    }

    return 0;
}

static inline bool element_is_subscribing(struct element *elem,
                      uint16_t addr)
{

    if (g_slist_find(elem->subscribe_l, GUINT_TO_POINTER(addr)))
              return true;

    return false;
}

static inline void element_subscribe(struct element *elem,
                     uint16_t addr)
{
    if (element_is_subscribing(elem, addr))
        return;

    g_slist_append(elem->subscribe_l, GUINT_TO_POINTER(addr));
}

struct element *element_by_address(uint16_t addr)
{
    uint8_t elem_index;

    /* get primary network */
    struct network *net = network_by_index(0);
    if (!net)
        return NULL;

    /* calculate index from base address */
    elem_index = addr - net->addr;
    if (elem_index < 0)
        return NULL;

    return element_by_index(elem_index);
}

struct element *element_by_index(int index)
{

    GSList *l;

    for (l = node.element_l; l != NULL; l = l->next) {
        struct element *elem = l->data;

        if (elem->index == index)
            return elem;
    }

    return NULL;
}
uint16_t returnBaseAddressFromElement(){
    struct network *net = network_by_index(0);
    if (!net)
        return NULL;
    GSList *l;
    uint16_t baseAddr=-1;
    for (l = node.element_l; l != NULL; l = l->next) {
        struct element *elem = l->data;

        if (elem->index >=0)
        {
            baseAddr =  net->addr+elem->index;
            break;
        }
    }
    return baseAddr;
}
struct element *element_create(int index)
{

    struct element *elem = g_new0(struct element, 1);

    if (element_by_index(index))
        return NULL;

    elem->index = index;

    node.element_l = g_slist_append(node.element_l, elem);

    return elem;
}

static int element_recv_msg(struct element *elem, uint16_t src,
                uint8_t *data, size_t dlen)
{

    uint32_t opcode = access_msg_get_opcode(data);
    GSList *m;

    /* The opcode belongs to the addressed model’s element. */
    for (m = elem->serv_model_l; m != NULL; m = m->next) {
        const struct server_model *model = m->data;
        const struct state *state;
        int i = 0;

        /* for each state */
        while ((state = model->states[i++])) {
            const struct amsg_desc *desc;
            int j = 0;

            /* for each supported rx message */
            while ((desc = state->rx[j++])) {
                if (desc->opcode == opcode) {

                    return 0;
                }
            }
        }
    }

    /* The model is bound to the application or device key that was used to
     * secure the transportation of the message
     */

    return 0;
}

int access_recv_msg(void *data, size_t len, uint16_t src, uint16_t dst)
{
    unsigned int addr_type = ADDR_TYPE(dst);
    int err = 0;
    char* data1=(char*) data;

    bool matchedAddress;
        processing_From_MessageType(data,len,src,dst);
        //return 0;
        matchedAddress=(!(element_by_address(dst)));

    if (!matchedAddress){
    g_message("Recv Access MSG (src=%04x, dst=%04x)", src, dst);
    }
    /* The destination address is set to one of the model’s element unicast
     * address or a group or virtual address for which the model’s element
     * is subscribed to, or the destination address is set to a fixed group
     * address of the primary element of the node as defined in Section
     */
    if (addr_type == ADDR_TYPE_UNICAST) {
        struct element *elem = element_by_address(dst);

        if (!elem)
            return -EADDRNOTAVAIL;

        err = element_recv_msg(elem, src, data, len);
    } else if ((addr_type == ADDR_TYPE_BROADCAST) ||
           (addr_type == ADDR_TYPE_GROUP) ||
           (addr_type == ADDR_TYPE_VIRTUAL)) {
        GSList *l;

        for (l = node.element_l; l != NULL; l = l->next) {
            if (element_is_subscribing(l->data, dst)) {
                element_recv_msg(l->data, src, data,
                         len);
            }
        }
    }

    return err;
}
void processing_From_MessageType(void *data, size_t len, uint16_t src,uint16_t dst){


    char* data1=(char*) data;

    char subbuff[len];
    memcpy( subbuff, &data1[0], len );
    subbuff[len] = '\0';
    updateLabel(subbuff);

    char * pch;
    pch = strtok (subbuff,",");


    if (strcmp(pch,"00")==0){
        oldRSSI=-1000;
        pch = strtok (NULL,",.! ");
        char s[88];
        sprintf(s,"%d",returnBaseAddressFromElement());
        int rssi=scanRSSI(pch);
        tmp_sendResponseMessage(0,returnBaseAddressFromElement(),returnBaseAddressFromElement(),src,rssi);

    }else if(strcmp(pch,"01")){
        if(pch!=NULL){
            pch = strtok (NULL,",.!\t ");

                        char *newADDR=pch ;
                        int newRSSI;

                         pch = strtok (NULL,",.!\t ");
                        newRSSI =atoi(pch);
                        if(newRSSI>oldRSSI){
                            oldADDR=newADDR;
                            oldRSSI=newRSSI;
                        }

                        if( pch != NULL ) {


                          }
            }

                char string1[]="Closest to -> ";


                char *string2=oldADDR;
                char *newString=malloc(sizeof(char)*(strlen(string1)+strlen(string2)));
                strcpy(newString,string1);
                strcat(newString,string2);
                updateLabel(newString);

                g_message(newString);
                free(newString);
                newString=NULL;
    }
}
int findRSSI_From_Address(char *dst){
    return scanRSSI(dst);
}
int register_server_model(struct server_model *model, int instance)
{

    struct element *elem = element_by_index(instance);

    if (!elem)
        return -EINVAL;

    /* TODO check state/msgs/opcodes */
    elem->serv_model_l = g_slist_append(elem->serv_model_l, model);

    g_message("Model [%s | 0x%08x] registered", model->desc, model->id);

    return 0;
}
