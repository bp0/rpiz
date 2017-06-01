/*
 * rpiz - https://github.com/bp0/rpiz
 * Copyright (C) 2017  Burt P. <pburt0@gmail.com>
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

#include "board.h"
#include "cpu.h"
#include "board_rpi.h"
#include "cpu_arm.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <gtk/gtk.h>
#pragma GCC diagnostic pop

const char about_text[] =
    "rpiz\n"
    "(c) 2017 Burt P. <pburt0@gmail.com>\n"
    "https://github.com/bp0/rpiz\n"
    "\n";

rpi_board *board;
arm_proc *proc;

static int rpiz_init(void) {
    board_init();
    cpu_init();
    board = rpi_board_new();
    proc = arm_proc_new();
    return 1;
}

static void rpiz_cleanup(void) {
    rpi_board_free(board);
    arm_proc_free(proc);
    board_cleanup();
    cpu_cleanup();
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

struct {
    gint timeout_id;
    gint interval_ms;
} refresh_timer;

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
   CPUFREQ_COL_KEY,
   CPUFREQ_COL_VALUE,
   CPUFREQ_COL_MIN,
   CPUFREQ_COL_MAX,
   CPUFREQ_N_COLUMNS
};

char *cpufreq_col_names[] = {
    "Core",
    "Cur",
    "Min",
    "Max"
};

enum
{
   FLAGS_COL_KEY,
   FLAGS_COL_CORE_COUNT,
   FLAGS_COL_VALUE,
   FLAGS_N_COLUMNS
};

char *flags_col_names[] = {
    "Flag",
    "Count",
    "Meaning"
};

GtkListStore *board_store;
GtkListStore *cpufreq_store;
GtkListStore *flags_store;
GtkWidget *board_view;
GtkWidget *cpufreq_view;
GtkWidget *flags_view;

static void init_list_stores(void) {
    board_store =
    gtk_list_store_new (BOARD_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

    cpufreq_store =
    gtk_list_store_new (CPUFREQ_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

    flags_store =
    gtk_list_store_new (FLAGS_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING);
}

#define BOARD_ADD(k, v) \
    gtk_list_store_append (board_store, &iter); \
    gtk_list_store_set (board_store, &iter,     \
                    BOARD_COL_KEY, (k),         \
                    BOARD_COL_VALUE, (v),       \
                    -1);

static void fill_board_list(void) {
    GtkTreeIter iter;
    gtk_list_store_clear (board_store);
    char soc_temp[64];
    sprintf(soc_temp, "%0.2f' C", rpi_soc_temp() );
    BOARD_ADD("Board", rpi_board_desc(board) );
    /* BOARD_ADD("Model", rpi_board_model(board) ); */
    /* BOARD_ADD("Revision", rpi_board_rev(board) ); */
    BOARD_ADD("Introduction", rpi_board_intro(board) );
    BOARD_ADD("Manufacturer", rpi_board_mfgby(board) );
    BOARD_ADD("RCode", rpi_board_rcode(board) );
    BOARD_ADD("Serial", rpi_board_serial(board) );
    BOARD_ADD("Memory (spec)", rpi_board_mem_spec(board) );
    BOARD_ADD("SOC (spec)", rpi_board_soc(board) );
    BOARD_ADD("SOC (reported)", arm_proc_name(proc) );
    BOARD_ADD("Processor", arm_proc_desc(proc) );
    BOARD_ADD("SOC Temp", soc_temp );
    BOARD_ADD("Overvolt", (rpi_board_overvolt(board)) ? "yes (warranty void!)" : "never" );
}

static void update_board_list(void) {
    GtkTreeIter  iter;
    gboolean     valid;
    gchar *key;
    char soc_temp[64];

    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(board_store), &iter);

    while (valid)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(board_store), &iter, BOARD_COL_KEY, &key, -1);
        if (g_strcmp0(key, "SOC Temp") == 0) {
            sprintf(soc_temp, "%0.2f' C", rpi_soc_temp() );
            gtk_list_store_set(board_store, &iter, BOARD_COL_VALUE, soc_temp, -1);
        }

        /* Get next row */
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(board_store), &iter);
    }
}

#define CPUFREQ_ADD(k, v, mi, ma) \
    gtk_list_store_append (cpufreq_store, &iter); \
    gtk_list_store_set (cpufreq_store, &iter,     \
                    CPUFREQ_COL_KEY, k,           \
                    CPUFREQ_COL_VALUE, v,         \
                    CPUFREQ_COL_MIN, mi,          \
                    CPUFREQ_COL_MAX, ma,          \
                    -1);

static void fill_cpufreq_list(void) {
    GtkTreeIter iter;
    gtk_list_store_clear (cpufreq_store);
    int cores = 0, c = 0;
    char id[16] = "", cur[24] = "", min[24] = "", max[24] = "";
    cores = arm_proc_cores(proc);
    for (c = 0; c < cores; c++) {
        sprintf(id, "%d", arm_proc_core_id(proc, c));
        sprintf(cur, "%0.2f MHz", (double)arm_proc_core_khz_cur(proc, c) / 1000);
        sprintf(min, "%0.2f MHz", (double)arm_proc_core_khz_min(proc, c) / 1000);
        sprintf(max, "%0.2f MHz", (double)arm_proc_core_khz_max(proc, c) / 1000);
        CPUFREQ_ADD(id, cur, min, max);
    }
}

static void update_cpufreq_list(void) {
    GtkTreeIter  iter;
    gboolean     valid;
    gchar *core_id;
    char cur[24] = "";
    int c;

    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(cpufreq_store), &iter);

    while (valid)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(cpufreq_store), &iter, CPUFREQ_COL_KEY, &core_id, -1);
        c = g_ascii_strtoll(core_id, NULL, 10);
        c = arm_proc_core_from_id(proc, c);

        sprintf(cur, "%0.2f MHz", (double)arm_proc_core_khz_cur(proc, c) / 1000);
        gtk_list_store_set(cpufreq_store, &iter, CPUFREQ_COL_VALUE, cur, -1);

        /* Get next row */
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(cpufreq_store), &iter);
    }
}

#define FLAGS_ADD(k, c, v) \
    gtk_list_store_append (flags_store, &iter); \
    gtk_list_store_set (flags_store, &iter,     \
                    FLAGS_COL_KEY, k,           \
                    FLAGS_COL_CORE_COUNT, c,    \
                    FLAGS_COL_VALUE, v,         \
                    -1);

static void fill_flags_list(void) {
    /*
    GtkTreeIter iter;
    gint i = 0, core_count = 0;
    gchar **all_flags;
    all_flags = g_strsplit(arm_flag_list(), " ", 0);
    while(all_flags[i] != NULL) {
        if (g_strcmp0(all_flags[i], "") != 0) {
            core_count = arm_proc_has_flag(proc, all_flags[i]);
            if (core_count) {
                FLAGS_ADD(all_flags[i], core_count, arm_flag_meaning(all_flags[i]) );
            }
        }
        i++;
    }
    g_strfreev(all_flags);
    */
    GtkTreeIter iter;
    gint i = 0, core_count = 0;
    gchar **all_flags;
    all_flags = g_strsplit(cpu_all_flags(), " ", 0);
    while(all_flags[i] != NULL) {
        if (g_strcmp0(all_flags[i], "") != 0) {
            core_count = cpu_has_flag(all_flags[i]);
            if (core_count) {
                FLAGS_ADD(all_flags[i], core_count, cpu_flag_meaning(all_flags[i]) );
            }
        }
        i++;
    }
    g_strfreev(all_flags);
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    widget = widget; /* to avoid a warning */
    event = event; /* to avoid a warning */
    data = data; /* to avoid a warning */
    return FALSE;
}

static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    widget = widget; /* to avoid a warning */
    data = data; /* to avoid a warning */
    g_source_remove(refresh_timer.timeout_id);
    rpiz_cleanup();
    gtk_main_quit();
}

static void add_notebook_page(const gchar *label, GtkWidget *notebook, GtkWidget *page_widget, gint border) {
    GtkWidget *lbl = gtk_label_new (label);
    gtk_container_set_border_width (GTK_CONTAINER (page_widget), border);
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

static void cpufreq_view_init(void) {
    gint i = 0;
    for(i = 0; i < CPUFREQ_N_COLUMNS; i++) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (cpufreq_col_names[i],
                                                       renderer,
                                                       "text", i,
                                                       NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (cpufreq_view), column);
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

static gboolean refresh_data(gpointer data) {
    data = data; /* to avoid a warning */
    update_board_list();
    update_cpufreq_list();
    return G_SOURCE_CONTINUE;
}

int main( int   argc,
          char *argv[] )
{
    rpiz_init();
    init_list_stores();
    fill_board_list();
    fill_cpufreq_list();
    fill_flags_list();

    /* dump data to console */
    gchar *summary_text;
    summary_text = rpiz_text_summary();
    printf("%s", summary_text);
    g_free(summary_text);

    GtkTextBuffer *about_text_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (about_text_buffer, about_text, -1);

    refresh_timer.interval_ms = 500;
    refresh_timer.timeout_id = g_timeout_add(refresh_timer.interval_ms, refresh_data, NULL);

    /* GUI */
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *mbox;

    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (window, "delete-event",
          G_CALLBACK (delete_event), NULL);
    g_signal_connect (window, "destroy",
          G_CALLBACK (destroy), NULL);

    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* notebook pages */
    GtkWidget *about_page = gtk_text_view_new_with_buffer(about_text_buffer);
    board_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (board_store));
    cpufreq_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (cpufreq_store));
    flags_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (flags_store));
    board_view_init();
    cpufreq_view_init();
    flags_view_init();

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_TOP);

    mbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mbox), board_view, TRUE, TRUE, 0); gtk_widget_show (board_view);
    gtk_box_pack_start (GTK_BOX (mbox), cpufreq_view, FALSE, FALSE, 5); gtk_widget_show (cpufreq_view);

    add_notebook_page("Board", notebook, mbox, 0);
    add_notebook_page("CPU Flags", notebook, flags_view, 10);
    add_notebook_page("About", notebook, about_page, 0);

    /* This packs the notebook into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_widget_show (notebook);

    gtk_window_resize (GTK_WINDOW (window), 450, 400);
    gtk_widget_show (window);

    gtk_main ();

    return 0;
}
