/*
 * 
 * Copyright (C) 2017  Burt P. (pburt0@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 */

#include "board_rpi.h"
#include "cpu_arm.h"
#include <gtk/gtk.h>

const char about_text[] =
    "rpiz\n"
    "(c) 2017 Burt P. <pburt0@gmail.com>\n"
    "https://github.com/bp0/rpiz\n"
    "\n";

rpi_board *board;
arm_proc *proc;

static int rpiz_init(void) {
    board = rpi_board_new();
    proc = arm_proc_new();
    return 1;
}

static void rpiz_cleanup(void) {
    rpi_board_free(board);
    arm_proc_free(proc);
}

static gchar* rpiz_text_summary(void) {
    return g_strdup_printf(
        "Board: %s\n"
        "Model: %s\n"
        "Revision: %s\n"
        "Introduction: %s\n"
        "Manufactured by: %s\n"
        "RCode: %s\n"
        "Serial Number: %s\n"
        "Memory (spec): %s\n"
        "SOC (spec): %s\n"
        "CPU: %s\n"
        "SOC Temp: %0.2f C\n"
        "%s",
        rpi_board_desc(board),
        rpi_board_model(board),
        rpi_board_rev(board),
        rpi_board_intro(board),
        rpi_board_mfgby(board),
        rpi_board_rcode(board),
        rpi_board_serial(board),
        rpi_board_mem_spec(board),
        rpi_board_soc(board),
        arm_proc_desc(proc),
        rpi_soc_temp(),
        "");
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
}

static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    rpiz_cleanup();
    gtk_main_quit();
}

static void add_notebook_page(const gchar *label, GtkWidget *notebook, GtkWidget *page_widget) {
    GtkWidget *lbl = gtk_label_new (label);
    gtk_container_set_border_width (GTK_CONTAINER (page_widget), 10);
    gtk_widget_set_size_request (page_widget, 100, 75);
    gtk_notebook_append_page(GTK_NOTEBOOK (notebook), page_widget, lbl);
    gtk_widget_show (page_widget);
}

int main( int   argc,
          char *argv[] )
{
    //gint i = 0;

    rpiz_init();
    
    /* dump data to console */
    gchar *summary_text;
    //gchar *core_text;
    summary_text = rpiz_text_summary();
    printf("%s", summary_text);
    GtkTextBuffer *summary_text_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (summary_text_buffer, summary_text, -1);
    g_free(summary_text);
    
    /*
    GtkTextBuffer *core_text_buffer[MAX_CORES];
    for(i = 0; i < rpi.core_count; i++) {
        core_text = rpi_core_summary(i);
        //g_printf("%s", core_text);
        core_text_buffer[i] = gtk_text_buffer_new (NULL);
        gtk_text_buffer_set_text (core_text_buffer[i], core_text, -1);
        g_free(core_text);
    }
    */

    GtkTextBuffer *about_text_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (about_text_buffer, about_text, -1);

    /* GUI */
    GtkWidget *window;
    GtkWidget *notebook;
    
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    g_signal_connect (window, "delete-event",
          G_CALLBACK (delete_event), NULL);
    g_signal_connect (window, "destroy",
          G_CALLBACK (destroy), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* notebook pages */
    GtkWidget *summary_page = gtk_text_view_new_with_buffer(summary_text_buffer);
    /* GtkWidget *core_page[MAX_CORES]; */
    GtkWidget *about_page = gtk_text_view_new_with_buffer(about_text_buffer);
        
    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    add_notebook_page("Summary", notebook, summary_page);
    /*
    for(i = 0; i < rpi.core_count; i++) {
        core_page[i] = gtk_text_view_new_with_buffer(core_text_buffer[i]);
        gchar *corelabel = g_strdup_printf("Core %d", i);
        add_notebook_page(corelabel, notebook, core_page[i]);
        g_free(corelabel);
    }
    */
    add_notebook_page("About", notebook, about_page);

    /* This packs the notebook into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_widget_show (notebook);    
    gtk_widget_show (window);
    
    gtk_main ();

    return 0;
}
