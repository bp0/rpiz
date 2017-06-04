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

arm_proc *proc; // TODO
rpiz_fields *all_fields;

static int rpiz_init(void) {
    rpiz_fields *bf, *pf;
    board_init();
    cpu_init();
    proc = arm_proc_new(); // TODO
    bf = board_fields();
    pf = cpu_fields();
    all_fields = fields_copy(bf, pf);
    return 1;
}

static void rpiz_cleanup(void) {
    arm_proc_free(proc); // TODO
    fields_free(all_fields);
    board_cleanup();
    cpu_cleanup();
}

struct {
    gint timeout_id;
    gint interval_ms;
} refresh_timer;

enum
{
   KV_COL_KEY,
   KV_COL_VALUE,
   KV_COL_TAG,
   KV_COL_LIVE,
   KV_N_COLUMNS
};

char *kv_col_names[] = {
    "Item",
    "Value",
    "Tag",
    "Live",
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

struct {
    GtkListStore *summary_store;
    GtkWidget *summary_view;
    GtkListStore *board_store;
    GtkWidget *board_view;
    GtkListStore *cpu_store;
    GtkWidget *cpu_view;
    GtkListStore *cpufreq_store;
    GtkWidget *cpufreq_view;
    GtkListStore *flags_store;
    GtkWidget *flags_view;
} gel;

static GtkListStore *kv_store_create() {
    return gtk_list_store_new (KV_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
}

#define KV_ADD(k, v, t, l) \
    gtk_list_store_append (store, &iter); \
    gtk_list_store_set (store, &iter,     \
                    KV_COL_KEY, (k),      \
                    KV_COL_VALUE, (v),    \
                    KV_COL_TAG, (t),      \
                    KV_COL_LIVE, (l),     \
                    -1);
static void kv_fill_store_by_fields(GtkListStore *store, char *prefix) {
    GtkTreeIter iter;
    gtk_list_store_clear (store);
    char *tag, *name, *value;
    int m, l;
    rpiz_fields *f = all_fields;
    while (f) {
        m = 1;
        if (prefix)
            if (!fields_tag_has_prefix(f, prefix))
                m = 0;

        if (m)
            if (fields_get(f, &tag, &name, &value)) {
                l = fields_islive(f, tag);
                KV_ADD(name, value, tag, l);
            }

        if (prefix)
            f = fields_next_with_tag_prefix(f, prefix);
    }
}

static void kv_view_update(GtkListStore *store) {
    GtkTreeIter  iter;
    gboolean     valid;
    gchar *tag, *value;
    int live;

    /* Get first row in list store */
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

    while (valid)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, KV_COL_LIVE, &live, -1);
        if (live) {
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, KV_COL_TAG, &tag, -1);
            fields_get_bytag(all_fields, tag, NULL, &value);
            gtk_list_store_set(store, &iter, KV_COL_VALUE, value, -1);
        }

        /* Get next row */
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
}

#define CPUFREQ_ADD(k, v, mi, ma) \
    gtk_list_store_append (gel.cpufreq_store, &iter); \
    gtk_list_store_set (gel.cpufreq_store, &iter,     \
                    CPUFREQ_COL_KEY, k,           \
                    CPUFREQ_COL_VALUE, v,         \
                    CPUFREQ_COL_MIN, mi,          \
                    CPUFREQ_COL_MAX, ma,          \
                    -1);

static void fill_cpufreq_list(void) {
    GtkTreeIter iter;
    gtk_list_store_clear (gel.cpufreq_store);
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
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gel.cpufreq_store), &iter);

    while (valid)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(gel.cpufreq_store), &iter, CPUFREQ_COL_KEY, &core_id, -1);
        c = g_ascii_strtoll(core_id, NULL, 10);
        c = arm_proc_core_from_id(proc, c);

        sprintf(cur, "%0.2f MHz", (double)arm_proc_core_khz_cur(proc, c) / 1000);
        gtk_list_store_set(gel.cpufreq_store, &iter, CPUFREQ_COL_VALUE, cur, -1);

        /* Get next row */
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(gel.cpufreq_store), &iter);
    }
}

#define FLAGS_ADD(k, c, v) \
    gtk_list_store_append (gel.flags_store, &iter); \
    gtk_list_store_set (gel.flags_store, &iter,     \
                    FLAGS_COL_KEY, k,           \
                    FLAGS_COL_CORE_COUNT, c,    \
                    FLAGS_COL_VALUE, v,         \
                    -1);

static void fill_flags_list(void) {
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

static void init_list_stores(void) {
    gel.summary_store = kv_store_create();
    gel.board_store = kv_store_create();
    gel.cpu_store = kv_store_create();

    gel.cpufreq_store =
    gtk_list_store_new (CPUFREQ_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

    gel.flags_store =
    gtk_list_store_new (FLAGS_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_STRING);
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

static void kv_view_init(GtkWidget *view) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint i = 0;
    /* only name and value */
    for(i = 0; i < 2; i++) {
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (kv_col_names[i],
                                                       renderer,
                                                       "text", i,
                                                       NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);
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
        gtk_tree_view_append_column (GTK_TREE_VIEW (gel.cpufreq_view), column);
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
        gtk_tree_view_append_column (GTK_TREE_VIEW (gel.flags_view), column);
    }
}

static gboolean refresh_data(gpointer data) {
    data = data; /* to avoid a warning */
    update_cpufreq_list();
    kv_view_update(gel.summary_store);
    kv_view_update(gel.board_store);
    kv_view_update(gel.cpu_store);
    return G_SOURCE_CONTINUE;
}

int main( int   argc,
          char *argv[] )
{
    rpiz_init();
    init_list_stores();
    kv_fill_store_by_fields(gel.summary_store, "summary.");
    kv_fill_store_by_fields(gel.board_store, "board.");
    kv_fill_store_by_fields(gel.cpu_store, "cpu.");
    fill_cpufreq_list();
    fill_flags_list();

    /* dump data to console */
    fields_dump(all_fields);

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

#define V(v, s) (v = gtk_tree_view_new_with_model (GTK_TREE_MODEL (s)))
    V(gel.summary_view, gel.summary_store);
    V(gel.board_view, gel.board_store);
    V(gel.cpu_view, gel.cpu_store);
    V(gel.cpufreq_view, gel.cpufreq_store);
    V(gel.flags_view, gel.flags_store);
    kv_view_init(gel.summary_view);
    kv_view_init(gel.board_view);
    kv_view_init(gel.cpu_view);
    cpufreq_view_init();
    flags_view_init();

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_TOP);

    mbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mbox), gel.summary_view, TRUE, TRUE, 0); gtk_widget_show (gel.summary_view);
    gtk_box_pack_start (GTK_BOX (mbox), gel.cpufreq_view, FALSE, FALSE, 5); gtk_widget_show (gel.cpufreq_view);

    add_notebook_page("Summary", notebook, mbox, 0);
    add_notebook_page("Board", notebook, gel.board_view, 10);
    add_notebook_page("CPU", notebook, gel.cpu_view, 10);
    add_notebook_page("CPU Flags", notebook, gel.flags_view, 10);
    add_notebook_page("About", notebook, about_page, 0);

    /* This packs the notebook into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_widget_show (notebook);

    gtk_window_resize (GTK_WINDOW (window), 450, 400);
    gtk_widget_show (window);

    gtk_main ();

    return 0;
}
