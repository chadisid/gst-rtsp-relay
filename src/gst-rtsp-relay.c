/*
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 * Author: Alessandro Decina <alessandro.d@gmail.com>
 */

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "gst-rtsp-relay-media-factory.h"

static gboolean
timeout (GstRTSPServer *server, gboolean ignored)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}

int
main(int argc, char **argv)
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPRelayMediaFactory *factory;
  gchar *name;
  GstRTSPUrl *local_url; 
  gchar *service;

  gst_init (&argc, &argv);

  if (argc != 3) {
    g_printerr ("Usage: %s [LOCAL_URL] [REMOTE_URL]\n", argv[0]);

    return 1;
  }

  if (gst_rtsp_url_parse (argv[1], &local_url) != GST_RTSP_OK) {
    g_printerr ("invalid local url");

    return 1;
  }

  loop = g_main_loop_new (NULL, FALSE);

  server = gst_rtsp_server_new ();
  service = g_strdup_printf ("%d", local_url->port);
  gst_rtsp_server_set_service (server, service);
  g_free (service);

  name = g_strdup_printf ("rtsp-relay-factory-%s", argv[2]);
  factory = gst_rtsp_relay_media_factory_new (argv[2]);
  g_object_set (factory, "timeout", 20 * GST_SECOND, NULL);
  g_object_set (factory, "latency", 300 * GST_MSECOND, NULL);
  g_free (name);

  gst_rtsp_media_factory_set_shared (GST_RTSP_MEDIA_FACTORY (factory), TRUE);
  mapping = gst_rtsp_server_get_media_mapping (server);
  gst_rtsp_media_mapping_add_factory (mapping, local_url->abspath,
      GST_RTSP_MEDIA_FACTORY (factory));
  g_object_unref (mapping);

  gst_rtsp_url_free (local_url);

  gst_rtsp_server_attach (server, NULL);

  g_timeout_add_seconds (10, (GSourceFunc) timeout, server); 
  /* start serving */
  g_main_loop_run (loop);

  return 0;
}
