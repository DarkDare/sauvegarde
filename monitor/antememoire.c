/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    antememoire.c
 *    This file is part of "Sauvegarde" project.
 *
 *    (C) Copyright 2014 - 2015 Olivier Delhomme
 *     e-mail : olivier.delhomme@free.fr
 *
 *    "Sauvegarde" is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    "Sauvegarde" is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with "Sauvegarde".  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file antememoire.c
 * Here on should find everything that will deal with the cache. All writes
 * to the cache should only occur in store_buffer_data()'s thread.
 */

#include "monitor.h"

/**
 * This function saves meta_data_t structure into the cache or to the
 * serveur's server (it then inserts hostname into the message that is
 * sent).
 * @param meta : the meta_data_t * structure to be saved.
 * @param main_struct : main structure of the program (contains pointers
 *        to the communication socket, the cache database connexion and the
 *        balanced binary tree that contains hashs.
 */
static void insert_meta_data_into_cache_or_send_to_serveur(meta_data_t *meta, main_struct_t *main_struct)
{
    gchar *json_str = NULL;

    if (main_struct != NULL && meta != NULL && meta->name != NULL)
        {
            json_str = convert_meta_data_to_json(meta, main_struct->hostname);

            print_debug("json string (%d bytes) is : %s\n", strlen(json_str), json_str);

            /* send message here */
            main_struct->comm->buffer = json_str;
            post_url(main_struct->comm, "/Server/meta.json");

            /* freeing json_str may only happen when the message has been received */
            json_str = free_variable(json_str);
            main_struct->comm->buffer = NULL;

            print_debug("Inserting into database cache file %s\n", meta->name);
            insert_file_into_cache(main_struct->database, meta, main_struct->hashs);
        }
}


/**
 * This function is a thread that is waiting to receive messages from
 * the checksum function and whose aim is to store somewhere the data
 * of a buffer that a been checksumed.
 * @param data : main_struct_t * structure.
 * @returns NULL to fullfill the template needed to create a GThread
 */
gpointer store_buffer_data(gpointer data)
{
    main_struct_t *main_struct = (main_struct_t *) data;
    capsule_t *capsule = NULL;

    if (main_struct != NULL)
        {


            if (main_struct->comm != NULL)
                {
                    get_url(main_struct->comm, "/Version");

                    do
                        {
                            capsule = g_async_queue_pop(main_struct->store_queue);
                            print_debug(_("A capsule (%d) has been received in store's thread\n"), capsule->command);

                            switch (capsule->command)
                                {
                                    case ENC_META_DATA:
                                        insert_meta_data_into_cache_or_send_to_serveur((meta_data_t *) capsule->data, main_struct);
                                        /* capsule = free_variable(capsule); */
                                    break;

                                    case ENC_END:
                                    break;

                                    default :
                                        print_debug(_("Default case !\n"));
                                    break;
                                }

                        }
                    while (capsule->command != ENC_END);

                    capsule = free_variable(capsule);
                }
            else
                {
                    print_error(__FILE__, __LINE__, _("Error while initializing libcurl.\n"));
                }
        }

    return NULL;
}
