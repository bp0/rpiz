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
        "CPU Description: %s\n"
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
        arm_proc_name(proc),
        arm_proc_desc(proc),
        rpi_soc_temp(),
        "");
}

enum
{
   BOARD_COL_KEY,
   BOARD_COL_VALUE,
   BOARD_N_COLUMNS
};

char *board_col_names[] = {
    "Item",
    "Value"
};

enum
{
   CPU_COL_KEY,
   CPU_COL_VALUE,
   CPU_COL_MIN,
   CPU_COL_MAX,
   CPU_N_COLUMNS
};

char *cpu_col_names[] = {
    "Core",
    "Cur",
    "Min",
    "Max"
};

enum
{
   FLAGS_COL_KEY,
   FLAGS_COL_EXISTS,
   FLAGS_COL_VALUE,
   FLAGS_N_COLUMNS
};

char *flags_col_names[] = {
    "Flag",
    "Present",
    "Meaning"
};

GtkListStore *board_store;
GtkListStore *cpu_store;
GtkListStore *flags_store;
GtkWidget *board_view;
GtkWidget *cpu_view;
GtkWidget *flags_view;

static void init_list_stores(void) {
    board_store = 
    gtk_list_store_new (BOARD_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

    cpu_store = 
    gtk_list_store_new (CPU_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

    flags_store = 
    gtk_list_store_new (FLAGS_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_BOOLEAN,
                        G_TYPE_STRING);
}

#define BOARD_ADD(k, v) \
    gtk_list_store_append (board_store, &iter); \
    gtk_list_store_set (board_store, &iter,     \
                    BOARD_COL_KEY, k,           \
                    BOARD_COL_VALUE, v,         \
                    -1);

static void fill_board_list(void) {
    GtkTreeIter iter;
    char soc_temp[64];
    sprintf(soc_temp, "%0.2f' C", rpi_soc_temp() );
    BOARD_ADD("Board", rpi_board_desc(board) );
    BOARD_ADD("Model", rpi_board_model(board) );
    BOARD_ADD("Revision", rpi_board_rev(board) );
    BOARD_ADD("Introduction", rpi_board_intro(board) );
    BOARD_ADD("Manufacturer", rpi_board_mfgby(board) );
    BOARD_ADD("RCode", rpi_board_rcode(board) );
    BOARD_ADD("Serial", rpi_board_serial(board) );
    BOARD_ADD("Memory (spec)", rpi_board_mem_spec(board) );
    BOARD_ADD("SOC (spec)", rpi_board_soc(board) );
    BOARD_ADD("SOC (reported)", arm_proc_name(proc) );
    BOARD_ADD("Processor", arm_proc_desc(proc) );
    BOARD_ADD("SOC Temp", soc_temp );
}

#define CPU_ADD(k, v, mi, ma) \
    gtk_list_store_append (cpu_store, &iter); \
    gtk_list_store_set (cpu_store, &iter,     \
                    CPU_COL_KEY, k,           \
                    CPU_COL_VALUE, v,         \
                    CPU_COL_MIN, mi,          \
                    CPU_COL_MAX, ma,          \
                    -1);

static void fill_cpu_list(void) {
    GtkTreeIter iter;
    int cores = 0, c = 0;
    char id[16] = "", cur[24] = "", min[24] = "", max[24] = "";
    cores = arm_proc_cores(proc);
    for (c = 0; c < cores; c++) {
        sprintf(id, "%d", arm_proc_core_id(proc, c));
        sprintf(cur, "%0.2f MHz", (double)arm_proc_core_khz_cur(proc, c) / 1000);
        sprintf(min, "%0.2f MHz", (double)arm_proc_core_khz_min(proc, c) / 1000);
        sprintf(max, "%0.2f MHz", (double)arm_proc_core_khz_max(proc, c) / 1000);
        CPU_ADD(id, cur, min, max);
    }
}

#define FLAGS_ADD(k, p, v) \
    gtk_list_store_append (flags_store, &iter); \
    gtk_list_store_set (flags_store, &iter,     \
                    FLAGS_COL_KEY, k,           \
                    FLAGS_COL_EXISTS, p,        \
                    FLAGS_COL_VALUE, v,         \
                    -1);

static void fill_flags_list(void) {
    GtkTreeIter iter;
    gint i = 0, present = 0;
    gchar **all_flags;
    all_flags = g_strsplit(arm_flag_list(), " ", 0);
    while(all_flags[i] != NULL) {
        if (g_strcmp0(all_flags[i], "") != 0) {
            present = arm_proc_has_flag(proc, all_flags[i]);
            FLAGS_ADD(all_flags[i], present, arm_flag_meaning(all_flags[i]) );
        }
        i++;
    }
    g_strfreev(all_flags);
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

static void board_view_init(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint i = 0;
    for(i = 0; i < BOARD_N_COLUMNS; i++) {
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (board_col_names[i],
                                                       renderer,
                                                       "text", i,
                                                       NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (board_view), column);
    }
}

static void cpu_view_init(void) {
    gint i = 0;
    for(i = 0; i < CPU_N_COLUMNS; i++) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (cpu_col_names[i],
                                                       renderer,
                                                       "text", i,
                                                       NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (cpu_view), column);
    }
}

static void flags_view_init(void) {
    gint i = 0;
    for(i = 0; i < FLAGS_N_COLUMNS; i++) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (flags_col_names[i],
                                                       renderer,
                                                       "text", i,
                                                       NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (flags_view), column);
    }
}

int main( int   argc,
          char *argv[] )
{
    rpiz_init();
    init_list_stores();
    fill_board_list();
    fill_cpu_list();
    fill_flags_list();
    
    /* dump data to console */
    gchar *summary_text;
    summary_text = rpiz_text_summary();
    printf("%s", summary_text);
    GtkTextBuffer *summary_text_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (summary_text_buffer, summary_text, -1);
    g_free(summary_text);

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
    GtkWidget *about_page = gtk_text_view_new_with_buffer(about_text_buffer);
    board_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (board_store));
    cpu_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (cpu_store));
    flags_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (flags_store));
    board_view_init();
    cpu_view_init();
    flags_view_init();

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    add_notebook_page("Board", notebook, board_view);
    add_notebook_page("CPU", notebook, cpu_view);
    add_notebook_page("Feature Flags", notebook, flags_view);
    add_notebook_page("Summary", notebook, summary_page);
    add_notebook_page("About", notebook, about_page);

    /* This packs the notebook into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_widget_show (notebook);    
    gtk_widget_show (window);
    
    gtk_main ();

    return 0;
}
