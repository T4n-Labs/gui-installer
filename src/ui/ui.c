/*
 * ui.c
 * Main UI Construction
 *
 * Navigation: GtkStack + custom sidebar
 */
#include "pinguin-installer.h"
#include <stdio.h>
#include <stdlib.h>
#include "logo.h"

/* ================================================================
 * Interne Sidebar-Daten
 * ================================================================ */

#define NUM_PAGES 7

static const char *PAGE_NAMES[NUM_PAGES] = {
    "welcome", "install_type", "partitions",
    "bootloader", "system", "users", "install"
};

static GtkWidget *g_sidebar_btns[NUM_PAGES];
static GtkWidget *g_stack      = NULL;   
static int        g_cur_page   = 0;
static GtkWidget *g_progress_bar_top = NULL;

/* ================================================================
 * Hilfsfunktionen für die Sidebar-Navigation
 * ================================================================ */

static void sidebar_update_buttons(int active_page) {
    for (int i = 0; i < NUM_PAGES; i++) {
        GtkStyleContext *ctx = gtk_widget_get_style_context(g_sidebar_btns[i]);
        gtk_style_context_remove_class(ctx, "sidebar-active");
        gtk_style_context_remove_class(ctx, "sidebar-done");
        if (i < active_page)
            gtk_style_context_add_class(ctx, "sidebar-done");
        else if (i == active_page)
            gtk_style_context_add_class(ctx, "sidebar-active");
    }
}

static void progress_update(int page) {
    if (!g_progress_bar_top) return;
    gdouble frac = (NUM_PAGES <= 1) ? 1.0 : (gdouble)page / (gdouble)(NUM_PAGES - 1);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progress_bar_top), frac);
}

static void goto_page(AppData *app, int page) {
    if (page < 0 || page >= NUM_PAGES) return;
    g_cur_page = page;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), PAGE_NAMES[page]);
    sidebar_update_buttons(page);
    progress_update(page);

    gtk_widget_set_sensitive(app->btn_back, (page > 0));
    gtk_widget_set_sensitive(app->btn_next, (page < NUM_PAGES - 1));
}

/* ================================================================
 * Navigation Callbacks
 * ================================================================ */

void on_next_clicked(GtkWidget *widget, AppData *app) {
    (void)widget;
    goto_page(app, g_cur_page + 1);
}

void on_back_clicked(GtkWidget *widget, AppData *app) {
    (void)widget;
    goto_page(app, g_cur_page - 1);
}

void on_page_changed(GtkNotebook *notebook, GtkWidget *page, guint page_num, AppData *app) {
    (void)notebook; (void)page; (void)page_num; (void)app;
}

static void on_sidebar_btn_clicked(GtkWidget *btn, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    for (int i = 0; i < NUM_PAGES; i++) {
        if (g_sidebar_btns[i] == btn) {
            goto_page(app, i);
            return;
        }
    }
}

/* ================================================================
 * Lokalisierung Strings
 * ================================================================ */
const char* get_loc(const char *key, int lang) {
    if (strcmp(key, "welcome_title") == 0)
        return lang == 1
        ? "<span size='xx-large' weight='bold'>Willkommen bei Pinguin-TV</span>"
        : "<span size='xx-large' weight='bold'>Welcome to Pinguin-TV-Installer</span>";
        
    /* HIER IST DER FIX FÜR DEN ENGLISCHEN TEXT (Zeilenumbruch \n hinzugefügt) */
    if (strcmp(key, "welcome_body") == 0)
        return lang == 1
        ? "<span size='large'>Pinguin-TV Installer ist ein inoffizieller Void Linux Installer,\n"
          "der eine leichte und einfache Installation bietet.</span>"
        : "<span size='large'>An unofficial Void Linux installer,\nlightweight and easy installation.</span>";

    if (strcmp(key, "disk_title") == 0)         return lang == 1 ? "Laufwerk:"          : "Disk:";
    if (strcmp(key, "disk_info") == 0)          return lang == 1
        ? "Hinweis: Partitionen können mit GParted geändert und unten konfiguriert werden."
        : "Note: You can modify the partitions with GParted and then configure them below.";
    if (strcmp(key, "part_mount") == 0)         return lang == 1 ? "Einhängepunkte:"    : "Mount Points:";
    if (strcmp(key, "btn_add") == 0)            return lang == 1 ? "Hinzufügen"         : "Add";
    if (strcmp(key, "btn_edit") == 0)           return lang == 1 ? "Bearbeiten"         : "Edit";
    if (strcmp(key, "btn_del") == 0)            return lang == 1 ? "Löschen"            : "Delete";
    if (strcmp(key, "btn_reset") == 0)          return lang == 1 ? "Einhängepunkte zurücksetzen" : "Reset mount points";
    if (strcmp(key, "btn_gparted") == 0)        return lang == 1 ? "Partitionieren (GParted)"    : "Partition (GParted)";

    if (strcmp(key, "tab_install_type") == 0)   return lang == 1 ? "Installationsart"   : "Install Type";
    if (strcmp(key, "install_type_title") == 0) return lang == 1 ? "Wähle die Installationsart:" : "Select installation type:";
    if (strcmp(key, "clean_install") == 0)      return lang == 1 ? "Neuinstallation (gesamte Festplatte löschen)" : "Clean Install (erase entire disk)";
    if (strcmp(key, "install_alongside") == 0)  return lang == 1 ? "Neben anderem System installieren (Größe ändern)" : "Install alongside another OS (resize)";
    if (strcmp(key, "clean_install_desc") == 0) return lang == 1
        ? "Alle Daten auf der gewählten Platte werden gelöscht und Partitionen automatisch erstellt."
        : "All data on the selected disk will be erased and partitions will be created automatically.";
    if (strcmp(key, "alongside_desc") == 0)     return lang == 1
        ? "Die bestehende Partition wird verkleinert, um Platz für das neue System zu schaffen."
        : "The existing partition will be resized to create free space for the new system.";
    if (strcmp(key, "manual_partitioning") == 0) return lang == 1 ? "Partitionen manuell konfigurieren" : "Configure partitions manually";
    if (strcmp(key, "existing_parts") == 0)     return lang == 1 ? "Vorhandene Partitionen" : "Existing Partitions";
    if (strcmp(key, "install_parts") == 0)      return lang == 1 ? "Installations-Partitionen" : "Installation Partitions";

    if (strcmp(key, "boot_detect") == 0)        return lang == 1 ? "Firmware wird erkannt..." : "Detecting firmware...";
    if (strcmp(key, "grub_install") == 0)       return lang == 1 ? "GRUB installieren auf:"   : "Install GRUB to:";

    if (strcmp(key, "hostname") == 0)           return lang == 1 ? "Rechnername:"       : "Hostname:";
    if (strcmp(key, "locale") == 0)             return lang == 1 ? "Sprache (Locale):"  : "Locale:";
    if (strcmp(key, "region") == 0)             return lang == 1 ? "Region:"            : "Region:";
    if (strcmp(key, "city") == 0)               return lang == 1 ? "Stadt:"             : "City:";

    if (strcmp(key, "root_pass") == 0)          return lang == 1 ? "Root-Passwort:"      : "Root Password:";
    if (strcmp(key, "user_acc_title") == 0)     return lang == 1 ? "Benutzerkonto:"      : "User Account:";
    if (strcmp(key, "fullname") == 0)           return lang == 1 ? "Vollständiger Name:" : "Full Name:";
    if (strcmp(key, "username") == 0)           return lang == 1 ? "Benutzername:"       : "Username:";
    if (strcmp(key, "password") == 0)           return lang == 1 ? "Passwort:"           : "Password:";
    if (strcmp(key, "confirm") == 0)            return lang == 1 ? "Bestätigen:"         : "Confirm:";
    if (strcmp(key, "autologin") == 0)          return lang == 1 ? "Auto-Login aktivieren" : "Enable Auto-Login";

    if (strcmp(key, "install_btn") == 0)        return lang == 1 ? "Installation starten" : "Start Installation";
    if (strcmp(key, "reboot_btn") == 0)         return lang == 1 ? "System neu starten"   : "Reboot System";
    if (strcmp(key, "back") == 0)               return lang == 1 ? "Zurück"  : "Back";
    if (strcmp(key, "next") == 0)               return lang == 1 ? "Weiter"  : "Next";

    if (strcmp(key, "tab_welcome") == 0)        return lang == 1 ? "Willkommen"   : "Welcome";
    if (strcmp(key, "tab_partitions") == 0)     return lang == 1 ? "Partitionen"  : "Partitions";
    if (strcmp(key, "tab_bootloader") == 0)     return lang == 1 ? "Bootloader"   : "Bootloader";
    if (strcmp(key, "tab_system") == 0)         return lang == 1 ? "System"       : "System";
    if (strcmp(key, "tab_users") == 0)          return lang == 1 ? "Benutzer"     : "Users";
    if (strcmp(key, "tab_install") == 0)        return lang == 1 ? "Installieren" : "Install";

    return key;
}

/* ================================================================
 * update_ui_language
 * ================================================================ */
void update_ui_language(AppData *app) {
    int lang = app->current_lang;

    const char *tab_keys[NUM_PAGES] = {
        "tab_welcome", "tab_install_type", "tab_partitions",
        "tab_bootloader", "tab_system", "tab_users", "tab_install"
    };
    
    for (int i = 0; i < NUM_PAGES; i++) {
        if (g_sidebar_btns[i])
            gtk_button_set_label(GTK_BUTTON(g_sidebar_btns[i]), get_loc(tab_keys[i], lang));
    }

    if (app->lbl_welcome_title) gtk_label_set_markup(GTK_LABEL(app->lbl_welcome_title), get_loc("welcome_title", lang));
    if (app->lbl_welcome_body) gtk_label_set_markup(GTK_LABEL(app->lbl_welcome_body), get_loc("welcome_body", lang));
    if (app->lbl_install_type_title) gtk_label_set_text(GTK_LABEL(app->lbl_install_type_title), get_loc("install_type_title", lang));
    if (app->radio_clean_install) gtk_button_set_label(GTK_BUTTON(app->radio_clean_install), get_loc("clean_install", lang));
    if (app->radio_alongside) gtk_button_set_label(GTK_BUTTON(app->radio_alongside), get_loc("install_alongside", lang));
    if (app->lbl_install_type_desc) gtk_label_set_text(GTK_LABEL(app->lbl_install_type_desc), get_loc("clean_install_desc", lang));
    if (app->lbl_disk_title) gtk_label_set_text(GTK_LABEL(app->lbl_disk_title), get_loc("disk_title", lang));
    if (app->lbl_disk_info) gtk_label_set_text(GTK_LABEL(app->lbl_disk_info), get_loc("disk_info", lang));
    if (app->lbl_part_mount) gtk_label_set_text(GTK_LABEL(app->lbl_part_mount), get_loc("part_mount", lang));
    if (app->btn_part_add) gtk_button_set_label(GTK_BUTTON(app->btn_part_add), get_loc("btn_add", lang));
    if (app->btn_part_edit) gtk_button_set_label(GTK_BUTTON(app->btn_part_edit), get_loc("btn_edit", lang));
    if (app->btn_part_del) gtk_button_set_label(GTK_BUTTON(app->btn_part_del), get_loc("btn_del", lang));
    if (app->btn_part_reset) gtk_button_set_label(GTK_BUTTON(app->btn_part_reset), get_loc("btn_reset", lang));
    if (app->btn_part_gparted) gtk_button_set_label(GTK_BUTTON(app->btn_part_gparted), get_loc("btn_gparted", lang));
    if (app->chk_manual_partitions) gtk_button_set_label(GTK_BUTTON(app->chk_manual_partitions), get_loc("manual_partitioning", lang));
    if (app->lbl_grub_install) gtk_label_set_text(GTK_LABEL(app->lbl_grub_install), get_loc("grub_install", lang));
    if (app->lbl_hostname) gtk_label_set_text(GTK_LABEL(app->lbl_hostname), get_loc("hostname", lang));
    if (app->lbl_locale) gtk_label_set_text(GTK_LABEL(app->lbl_locale), get_loc("locale", lang));
    if (app->lbl_region) gtk_label_set_text(GTK_LABEL(app->lbl_region), get_loc("region", lang));
    if (app->lbl_city) gtk_label_set_text(GTK_LABEL(app->lbl_city), get_loc("city", lang));
    if (app->lbl_root_pass) gtk_label_set_text(GTK_LABEL(app->lbl_root_pass), get_loc("root_pass", lang));
    if (app->lbl_user_account) gtk_label_set_text(GTK_LABEL(app->lbl_user_account), get_loc("user_acc_title", lang));
    if (app->lbl_fullname) gtk_label_set_text(GTK_LABEL(app->lbl_fullname), get_loc("fullname", lang));
    if (app->lbl_username) gtk_label_set_text(GTK_LABEL(app->lbl_username), get_loc("username", lang));
    if (app->lbl_user_pass) gtk_label_set_text(GTK_LABEL(app->lbl_user_pass), get_loc("password", lang));
    if (app->lbl_user_confirm) gtk_label_set_text(GTK_LABEL(app->lbl_user_confirm), get_loc("confirm", lang));
    if (app->chk_autologin) gtk_button_set_label(GTK_BUTTON(app->chk_autologin), get_loc("autologin", lang));

    if (!app->installing) {
        gtk_button_set_label(GTK_BUTTON(app->btn_install), get_loc("install_btn", lang));
    }
    gtk_button_set_label(GTK_BUTTON(app->btn_back), get_loc("back", lang));
    gtk_button_set_label(GTK_BUTTON(app->btn_next), get_loc("next", lang));
}

/* ================================================================
 * CSS (Dark Mode)
 * ================================================================ */
void load_custom_css() {
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css_data =
    "window { background-color: #242424; color: #eeeeee; }"
    "label { color: #eeeeee; }"
    ".sidebar {"
    "  background-color: #1a1a1a;"
    "  border-right: 1px solid #111111;"
    "}"
    ".sidebar-logo-box {"
    "  padding: 16px 14px 12px 14px;"
    "  border-bottom: 1px solid #2a2a2a;"
    "}"
    ".sidebar-logo-label {"
    "  color: #ffffff;"
    "  font-size: 13px;"
    "  font-weight: bold;"
    "}"
    ".sidebar-logo-sub {"
    "  color: #888888;"
    "  font-size: 10px;"
    "}"
    ".sidebar-btn {"
    "  margin: 0;"         /* HIER IST DER FIX FÜR DAS CSS-PADDING */
    "  background: transparent;"
    "  border: none;"
    "  border-radius: 0;"
    "  color: #999999;"
    "  font-size: 12px;"
    "  padding: 10px 16px;"
    "  box-shadow: none;"
    "}"
    ".sidebar-btn:hover {"
    "  background-color: #2a2a2a;"
    "  color: #ffffff;"
    "}"
    ".sidebar-active {"
    "  background-color: #2a2a2a;"
    "  color: #ffffff;"
    "  border-left: 3px solid #1D9E75;"
    "  padding-left: 13px;"
    "}"
    ".sidebar-done {"
    "  color: #1D9E75;"
    "}"
    ".top-progress trough {"
    "  min-height: 4px;"
    "  border-radius: 0;"
    "  background-color: #333333;"
    "}"
    ".top-progress progress {"
    "  min-height: 4px;"
    "  border-radius: 0;"
    "  background-color: #1D9E75;"
    "}"
    "progressbar trough { min-height: 6px; border-radius: 3px; background: #333333; }"
    "progressbar progress { min-height: 6px; border-radius: 3px; background: #1D9E75; }"
    "button {"
    "  border-radius: 6px;"
    "  padding: 6px 16px;"
    "  border: 1px solid #444444;"
    "  background: #333333;"
    "  color: #eeeeee;"
    "}"
    "button:hover { background: #4a4a4a; border-color: #666666; }"
    "button.suggested-action { background: #1D9E75; color: white; border-color: #1D9E75; }"
    "button.suggested-action:hover { background: #158562; }"
    "button.destructive-action { background: #c0392b; color: white; border-color: #a93226; }"
    "button.destructive-action:hover { background: #a93226; }"
    "entry { border-radius: 6px; border: 1px solid #444444; padding: 6px 10px; background: #1e1e1e; color: #eeeeee; }"
    "entry:focus { border-color: #1D9E75; }"
    "frame { border-radius: 8px; border: 1px solid #444444; padding: 4px; }"
    "frame > border { border-radius: 8px; border: 1px solid #444444; }"
    "frame label { color: #aaaaaa; font-weight: bold; }"
    "textview { background: #111111; color: #7ee787; font-family: monospace; font-size: 10pt; border-radius: 8px; }"
    "textview text { background: #111111; color: #7ee787; }"
    ".nav-bar {"
    "  background-color: #1a1a1a;"
    "  border-top: 1px solid #111111;"
    "  padding: 8px 14px;"
    "}";

    GError *error = NULL;
    gtk_css_provider_load_from_data(provider, css_data, -1, &error);
    if (error) {
        g_printerr("CSS Fehler: %s\n", error->message);
        g_error_free(error);
    } else {
        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

/* ================================================================
 * Sprachschalter / Fertig-Dialog
 * ================================================================ */
void on_lang_toggled(GtkWidget *widget, AppData *app) {
    app->current_lang = (app->current_lang == 0) ? 1 : 0;
    const char *lbl = (app->current_lang == 0) ? "English" : "Deutsch";
    gtk_button_set_label(GTK_BUTTON(widget), lbl);
    update_ui_language(app);
}

static void set_ui_finished_safe(gpointer data) {
    AppData *app = (AppData *)data;

    gtk_widget_set_sensitive(app->btn_back, FALSE);
    gtk_widget_set_sensitive(app->btn_next, FALSE);
    for (int i = 0; i < NUM_PAGES; i++)
        if (g_sidebar_btns[i])
            gtk_widget_set_sensitive(g_sidebar_btns[i], FALSE);

    gtk_button_set_label(GTK_BUTTON(app->btn_install),
                         get_loc("reboot_btn", app->current_lang));
    g_signal_handlers_disconnect_by_func(app->btn_install, G_CALLBACK(start_installation), app);
    g_signal_connect(app->btn_install, "clicked", G_CALLBACK(on_reboot_clicked), app);
    gtk_widget_set_sensitive(app->btn_install, TRUE);

    GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
                                            GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                                            app->current_lang == 1
                                            ? "PINGUIN-TV Linux ist BEREIT!!!"
                                            : "PINGUIN-TV Linux is READY!!!");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg),
                                             app->current_lang == 1
                                             ? "Installation erfolgreich abgeschlossen.\nDas System kann nun neu gestartet werden."
                                             : "Installation completed successfully.\nThe system is ready to restart.");

    GtkWidget *btn_reboot = gtk_dialog_add_button(GTK_DIALOG(dlg),
                                                  app->current_lang == 1 ? "JETZT NEU STARTEN" : "REBOOT NOW",
                                                  GTK_RESPONSE_ACCEPT);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_reboot), "suggested-action");
    g_signal_connect(dlg, "response", G_CALLBACK(on_popup_reboot), NULL);
    gtk_widget_show_all(dlg);
}

static gboolean set_ui_finished_wrapper(gpointer data) {
    set_ui_finished_safe(data);
    return FALSE;
}

void set_ui_finished(AppData *app) { 
    g_idle_add(set_ui_finished_wrapper, app); 
}

/* ================================================================
 * Formular-Helfer
 * ================================================================ */
GtkWidget* create_form_row(const gchar *label_text, GtkWidget **entry_ptr, GtkWidget **label_ptr) {
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_bottom(hbox, 5);
    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    if (label_ptr) *label_ptr = label;
    *entry_ptr = gtk_entry_new();
    gtk_widget_set_hexpand(*entry_ptr, TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), *entry_ptr, TRUE, TRUE, 0);
    return hbox;
}

GtkWidget* create_vertical_input(const gchar *label_text, GtkWidget **entry_ptr, GtkWidget **label_ptr) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_bottom(vbox, 10);
    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    if (label_ptr) *label_ptr = label;
    *entry_ptr = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), *entry_ptr, FALSE, FALSE, 0);
    return vbox;
}

/* ================================================================
 * Seiten-Konstruktoren
 * ================================================================ */

static GtkWidget* create_welcome_page(AppData *app) {
    GtkWidget *align_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(align_box, 40);
    gtk_widget_set_margin_bottom(align_box, 40);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
    if (gdk_pixbuf_loader_write(loader, logo_png, logo_png_len, NULL)) {
        gdk_pixbuf_loader_close(loader, NULL);
        GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
        if (pixbuf) {
            int w = gdk_pixbuf_get_width(pixbuf);
            int h = gdk_pixbuf_get_height(pixbuf);
            GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 200, 200 * h / w, GDK_INTERP_BILINEAR);
            GtkWidget *img = gtk_image_new_from_pixbuf(scaled);
            gtk_widget_set_halign(img, GTK_ALIGN_CENTER);
            gtk_widget_set_margin_bottom(img, 20);
            gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0);
            g_object_unref(scaled);
        }
    }
    g_object_unref(loader);

    app->lbl_welcome_title = gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL(app->lbl_welcome_title), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), app->lbl_welcome_title, FALSE, FALSE, 0);

    app->lbl_welcome_body = gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL(app->lbl_welcome_body), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(app->lbl_welcome_body), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(app->lbl_welcome_body), 60);
    gtk_label_set_selectable(GTK_LABEL(app->lbl_welcome_body), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), app->lbl_welcome_body, FALSE, FALSE, 10);

    GtkWidget *btn_lang = gtk_button_new_with_label("English");
    gtk_widget_set_halign(btn_lang, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btn_lang, 20);
    g_signal_connect(btn_lang, "clicked", G_CALLBACK(on_lang_toggled), app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_lang, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(align_box), vbox, TRUE, FALSE, 0);
    return align_box;
}

static GtkWidget* create_install_type_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(page), 20);

    app->lbl_install_type_title = gtk_label_new("Select installation type:");
    gtk_label_set_xalign(GTK_LABEL(app->lbl_install_type_title), 0.0);
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.2));
    gtk_label_set_attributes(GTK_LABEL(app->lbl_install_type_title), attrs);
    pango_attr_list_unref(attrs);
    gtk_box_pack_start(GTK_BOX(page), app->lbl_install_type_title, FALSE, FALSE, 0);

    app->radio_clean_install = gtk_radio_button_new_with_label(NULL, "Clean Install (erase entire disk)");
    gtk_box_pack_start(GTK_BOX(page), app->radio_clean_install, FALSE, FALSE, 0);

    app->radio_alongside = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(app->radio_clean_install), "Install alongside another OS (resize)");
    gtk_box_pack_start(GTK_BOX(page), app->radio_alongside, FALSE, FALSE, 0);

    app->lbl_install_type_desc = gtk_label_new(
        "All data on the selected disk will be erased and partitions will be created automatically.");
    gtk_label_set_xalign(GTK_LABEL(app->lbl_install_type_desc), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(app->lbl_install_type_desc), TRUE);
    gtk_widget_set_margin_top(app->lbl_install_type_desc, 10);
    gtk_widget_set_margin_start(app->lbl_install_type_desc, 10);
    gtk_box_pack_start(GTK_BOX(page), app->lbl_install_type_desc, FALSE, FALSE, 0);

    g_signal_connect(app->radio_clean_install, "toggled", G_CALLBACK(on_install_type_changed), app);
    g_signal_connect(app->radio_alongside, "toggled", G_CALLBACK(on_install_type_changed), app);
    return page;
}

static GtkWidget* create_partitions_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(page), 10);

    GtkWidget *hbox_disk = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(page), hbox_disk, FALSE, FALSE, 0);

    app->disk_combo = gtk_combo_box_new();
    GtkListStore *disk_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_combo_box_set_model(GTK_COMBO_BOX(app->disk_combo), GTK_TREE_MODEL(disk_store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(app->disk_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(app->disk_combo), renderer, "text", 0);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(app->disk_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(app->disk_combo), renderer, "text", 1);
    
    g_signal_connect(app->disk_combo, "changed", G_CALLBACK(on_disk_changed), app);

    app->lbl_disk_title = gtk_label_new("Disk:");
    gtk_box_pack_start(GTK_BOX(hbox_disk), app->lbl_disk_title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_disk), app->disk_combo, TRUE, TRUE, 0);

    app->btn_part_gparted = gtk_button_new_with_label("Partition (GParted)");
    g_signal_connect(app->btn_part_gparted, "clicked", G_CALLBACK(launch_gparted), app);
    gtk_box_pack_start(GTK_BOX(hbox_disk), app->btn_part_gparted, FALSE, FALSE, 0);

    app->lbl_disk_info = gtk_label_new(
        "Note: You can modify the partitions with GParted and then configure them below.");
    gtk_label_set_line_wrap(GTK_LABEL(app->lbl_disk_info), TRUE);
    gtk_label_set_xalign(GTK_LABEL(app->lbl_disk_info), 0.0);
    gtk_box_pack_start(GTK_BOX(page), app->lbl_disk_info, FALSE, FALSE, 0);

    GtkWidget *frame_existing = gtk_frame_new("Existing Partitions");
    gtk_box_pack_start(GTK_BOX(page), frame_existing, TRUE, TRUE, 0);

    app->existing_part_list = gtk_tree_view_new();
    init_existing_partitions_view(app);

    GtkWidget *scroll_existing = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll_existing), 100);
    gtk_widget_set_vexpand(scroll_existing, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll_existing), app->existing_part_list);
    gtk_container_add(GTK_CONTAINER(frame_existing), scroll_existing);

    app->chk_manual_partitions = gtk_check_button_new_with_label("Configure partitions manually");
    g_signal_connect(app->chk_manual_partitions, "toggled", G_CALLBACK(on_manual_check_toggled), app);
    gtk_box_pack_start(GTK_BOX(page), app->chk_manual_partitions, FALSE, FALSE, 0);

    app->frame_install_parts = gtk_frame_new("Installation Partitions");
    gtk_box_pack_start(GTK_BOX(page), app->frame_install_parts, TRUE, TRUE, 0);

    GtkWidget *vbox_install = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_install), 5);
    gtk_container_add(GTK_CONTAINER(app->frame_install_parts), vbox_install);

    GtkWidget *hbox_pm = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    app->lbl_part_mount = gtk_label_new("Mount Points:");
    gtk_box_pack_start(GTK_BOX(hbox_pm), app->lbl_part_mount, FALSE, FALSE, 0);

    app->btn_part_add = gtk_button_new_with_label("Add");
    g_signal_connect(app->btn_part_add, "clicked", G_CALLBACK(on_add_partition_clicked), app);
    gtk_box_pack_start(GTK_BOX(hbox_pm), app->btn_part_add, FALSE, FALSE, 0);

    app->btn_part_edit = gtk_button_new_with_label("Edit");
    g_signal_connect(app->btn_part_edit, "clicked", G_CALLBACK(on_edit_partition_clicked), app);
    gtk_box_pack_start(GTK_BOX(hbox_pm), app->btn_part_edit, FALSE, FALSE, 0);

    app->btn_part_del = gtk_button_new_with_label("Delete");
    g_signal_connect(app->btn_part_del, "clicked", G_CALLBACK(on_delete_partition_clicked), app);
    gtk_box_pack_start(GTK_BOX(hbox_pm), app->btn_part_del, FALSE, FALSE, 0);

    app->btn_part_reset = gtk_button_new_with_label("Reset mount points");
    g_signal_connect(app->btn_part_reset, "clicked", G_CALLBACK(on_reset_partitions_clicked), app);
    gtk_box_pack_start(GTK_BOX(hbox_pm), app->btn_part_reset, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox_install), hbox_pm, FALSE, FALSE, 0);

    app->mount_list = gtk_tree_view_new();
    open_partition_manager(app->mount_list, app);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 100);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), app->mount_list);
    gtk_box_pack_start(GTK_BOX(vbox_install), scrolled, TRUE, TRUE, 0);

    gtk_widget_set_sensitive(app->frame_install_parts, FALSE);
    return page;
}

static GtkWidget* create_bootloader_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(page), 15);

    app->lbl_boot_status = gtk_label_new("Detecting firmware...");
    app->label_boot_status = app->lbl_boot_status;
    gtk_box_pack_start(GTK_BOX(page), app->lbl_boot_status, FALSE, FALSE, 0);

    GtkWidget *hbox_grub = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    app->lbl_grub_install = gtk_label_new("Install GRUB to:");
    gtk_box_pack_start(GTK_BOX(hbox_grub), app->lbl_grub_install, FALSE, FALSE, 0);
    app->grub_disk_combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(hbox_grub), app->grub_disk_combo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page), hbox_grub, FALSE, FALSE, 0);

    return page;
}

static GtkWidget* create_system_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(page), 15);

    gtk_box_pack_start(GTK_BOX(page),
                       create_form_row("Hostname:", &app->hostname_entry, &app->lbl_hostname),
                       FALSE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(app->hostname_entry), "pinguin-tv");

    GtkWidget *vbox_loc = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_bottom(vbox_loc, 10);
    app->lbl_locale = gtk_label_new("Locale:");
    gtk_label_set_xalign(GTK_LABEL(app->lbl_locale), 0.5);
    app->locale_combo = gtk_combo_box_text_new();
    
    FILE *fp = fopen("/etc/default/libc-locales", "r");
    int count = 0, default_idx = 0;
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (line[0] == '\n' || !strstr(line, "UTF-8")) continue;
            char *p = line;
            while (*p == '#' || *p == ' ' || *p == '\t') p++;
            char name[128]; int i = 0;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n' && i < (int)sizeof(name) - 1)
                name[i++] = *p++;
            name[i] = '\0';
            if (i == 0) continue;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->locale_combo), name);
            if (g_strcmp0(name, "en_US.UTF-8") == 0) default_idx = count;
            count++;
        }
        fclose(fp);
    }
    
    if (count == 0) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->locale_combo), "en_US.UTF-8");
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->locale_combo), default_idx);
    
    gtk_box_pack_start(GTK_BOX(vbox_loc), app->lbl_locale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_loc), app->locale_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page), vbox_loc, FALSE, FALSE, 0);

    GtkWidget *vbox_tz = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_bottom(vbox_tz, 10);
    app->lbl_region = gtk_label_new("Region:");
    gtk_label_set_xalign(GTK_LABEL(app->lbl_region), 0.5);

    GtkWidget *hbox_tz = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    app->tz_area_combo = gtk_combo_box_text_new();
    const char *areas[] = {
        "Africa","America","Antarctica","Arctic","Asia",
        "Atlantic","Australia","Europe","Indian","Pacific", NULL
    };
    
    for (int i = 0; areas[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->tz_area_combo), areas[i]);
    }
    
    g_signal_connect(app->tz_area_combo, "changed", G_CALLBACK(on_timezone_area_changed), app);
    app->tz_city_combo = gtk_combo_box_text_new();
    
    gtk_box_pack_start(GTK_BOX(hbox_tz), app->tz_area_combo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_tz), app->tz_city_combo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_tz), app->lbl_region, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_tz), hbox_tz, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page), vbox_tz, FALSE, FALSE, 0);

    return page;
}

static GtkWidget* create_users_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(page), 15);

    gtk_box_pack_start(GTK_BOX(page),
                       create_vertical_input("Root Password:", &app->root_pass_entry, &app->lbl_root_pass),
                       FALSE, FALSE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(app->root_pass_entry), FALSE);

    gtk_box_pack_start(GTK_BOX(page),
                       create_vertical_input("Full Name:", &app->user_fullname_entry, &app->lbl_fullname),
                       FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(page),
                       create_vertical_input("Username:", &app->user_login_entry, &app->lbl_username),
                       FALSE, FALSE, 0);
    g_signal_connect(app->user_login_entry, "insert-text", G_CALLBACK(on_insert_text_username), NULL);

    gtk_box_pack_start(GTK_BOX(page),
                       create_vertical_input("Password:", &app->user_pass_entry, &app->lbl_user_pass),
                       FALSE, FALSE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(app->user_pass_entry), FALSE);

    gtk_box_pack_start(GTK_BOX(page),
                       create_vertical_input("Confirm:", &app->user_pass_confirm_entry, &app->lbl_user_confirm),
                       FALSE, FALSE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(app->user_pass_confirm_entry), FALSE);

    app->autologin_check = gtk_check_button_new_with_label("Enable Auto-Login");
    app->chk_autologin = app->autologin_check;
    gtk_widget_set_halign(app->autologin_check, GTK_ALIGN_CENTER);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->autologin_check), TRUE);
    gtk_box_pack_start(GTK_BOX(page), app->autologin_check, FALSE, FALSE, 10);

    return page;
}

static GtkWidget* create_install_page(AppData *app) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(page), 15);

    app->console_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->console_text), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), app->console_text);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(page), scroll, TRUE, TRUE, 0);

    app->progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(page), app->progress_bar, FALSE, FALSE, 0);

    app->btn_install = gtk_button_new_with_label("Start Installation");
    gtk_widget_set_size_request(app->btn_install, -1, 46);
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_install), "destructive-action");
    g_signal_connect(app->btn_install, "clicked", G_CALLBACK(start_installation), app);
    gtk_box_pack_start(GTK_BOX(page), app->btn_install, FALSE, FALSE, 0);

    return page;
}

/* ================================================================
 * build_ui
 * ================================================================ */
void build_ui(AppData *app) {
    load_custom_css();
    app->current_lang = 0;

    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Pinguin Installer – Void Linux");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 560);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbufLoader *icon_loader = gdk_pixbuf_loader_new();
    if (gdk_pixbuf_loader_write(icon_loader, logo_png, logo_png_len, NULL)) {
        gdk_pixbuf_loader_close(icon_loader, NULL);
        GdkPixbuf *icon = gdk_pixbuf_loader_get_pixbuf(icon_loader);
        if (icon) gtk_window_set_icon(GTK_WINDOW(app->window), icon);
    }
    g_object_unref(icon_loader);

    GtkWidget *root_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), root_vbox);

    GtkWidget *h_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(root_vbox), h_box, TRUE, TRUE, 0);

    /* ===== SIDEBAR ===== */
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(sidebar, 185, -1);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");
    gtk_box_pack_start(GTK_BOX(h_box), sidebar, FALSE, FALSE, 0);

    GtkWidget *logo_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(logo_box), "sidebar-logo-box");

    GdkPixbufLoader *sb_loader = gdk_pixbuf_loader_new();
    if (gdk_pixbuf_loader_write(sb_loader, logo_png, logo_png_len, NULL)) {
        gdk_pixbuf_loader_close(sb_loader, NULL);
        GdkPixbuf *pb = gdk_pixbuf_loader_get_pixbuf(sb_loader);
        if (pb) {
            int w = gdk_pixbuf_get_width(pb);
            int h = gdk_pixbuf_get_height(pb);
            GdkPixbuf *sc = gdk_pixbuf_scale_simple(pb, 36, 36 * h / w, GDK_INTERP_BILINEAR);
            GtkWidget *sb_img = gtk_image_new_from_pixbuf(sc);
            gtk_widget_set_halign(sb_img, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(logo_box), sb_img, FALSE, FALSE, 0);
            g_object_unref(sc);
        }
    }
    g_object_unref(sb_loader);

    GtkWidget *lbl_app = gtk_label_new("Pinguin-TV");
    gtk_label_set_xalign(GTK_LABEL(lbl_app), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_app), "sidebar-logo-label");
    gtk_box_pack_start(GTK_BOX(logo_box), lbl_app, FALSE, FALSE, 0);

    GtkWidget *lbl_sub = gtk_label_new("Void Linux Installer");
    gtk_label_set_xalign(GTK_LABEL(lbl_sub), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_sub), "sidebar-logo-sub");
    gtk_box_pack_start(GTK_BOX(logo_box), lbl_sub, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(sidebar), logo_box, FALSE, FALSE, 0);

    const char *step_labels[NUM_PAGES] = {
        "Welcome", "Install Type", "Partitions",
        "Bootloader", "System", "Users", "Install"
    };
    for (int i = 0; i < NUM_PAGES; i++) {
        GtkWidget *btn = gtk_button_new_with_label(step_labels[i]);
        /* HIER IST DER FIX: gtk_widget_set_hexpand(btn, TRUE); WURDE ENTFERNT */
        gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);
        gtk_widget_set_halign(btn, GTK_ALIGN_FILL);
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "sidebar-btn");
        GtkWidget *lbl = gtk_bin_get_child(GTK_BIN(btn));
        if (GTK_IS_LABEL(lbl)) gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);

        g_signal_connect(btn, "clicked", G_CALLBACK(on_sidebar_btn_clicked), app);
        gtk_box_pack_start(GTK_BOX(sidebar), btn, FALSE, FALSE, 0);
        g_sidebar_btns[i] = btn;
    }

    /* ===== CONTENT ===== */
    GtkWidget *content_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(h_box), content_vbox, TRUE, TRUE, 0);

    g_progress_bar_top = gtk_progress_bar_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(g_progress_bar_top), "top-progress");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g_progress_bar_top), 0.0);
    gtk_box_pack_start(GTK_BOX(content_vbox), g_progress_bar_top, FALSE, FALSE, 0);

    g_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(g_stack), 180);
    gtk_box_pack_start(GTK_BOX(content_vbox), g_stack, TRUE, TRUE, 0);

    app->notebook = g_stack;

    gtk_stack_add_named(GTK_STACK(g_stack), create_welcome_page(app), PAGE_NAMES[0]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_install_type_page(app), PAGE_NAMES[1]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_partitions_page(app), PAGE_NAMES[2]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_bootloader_page(app), PAGE_NAMES[3]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_system_page(app), PAGE_NAMES[4]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_users_page(app), PAGE_NAMES[5]);
    gtk_stack_add_named(GTK_STACK(g_stack), create_install_page(app), PAGE_NAMES[6]);

    /* ===== FOOTER-NAVIGATIONSLEISTE ===== */
    GtkWidget *nav_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(nav_bar), "nav-bar");

    app->btn_back = gtk_button_new_with_label("Back");
    gtk_widget_set_sensitive(app->btn_back, FALSE);
    g_signal_connect(app->btn_back, "clicked", G_CALLBACK(on_back_clicked), app);

    app->btn_next = gtk_button_new_with_label("Next");
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_next), "suggested-action");
    g_signal_connect(app->btn_next, "clicked", G_CALLBACK(on_next_clicked), app);

    gtk_box_pack_start(GTK_BOX(nav_bar), app->btn_back, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(nav_bar), app->btn_next, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(root_vbox), nav_bar, FALSE, FALSE, 0);

    update_ui_language(app);
    goto_page(app, 0);

    gtk_widget_show_all(app->window);
}