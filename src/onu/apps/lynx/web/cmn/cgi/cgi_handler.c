#include "cgi_system.h"
#include "cgi_upgrade.h"

CGI_HTTPD_HANDLER_REG("/cgi-bin/reset_onu.cgi", cgi_sys_reset);
CGI_HTTPD_HANDLER_REG("/cgi-bin/get_switch_type.cgi", cgi_switch_type_get);
CGI_HTTPD_HANDLER_REG("/cgi-bin/get_wlb_image.cgi", cgi_get_wlb_image);
CGI_HTTPD_HANDLER_REG("/cgi-bin/get_mif_image.cgi", cgi_get_mif_image);
#ifdef HAVE_FLASH_FS
CGI_HTTPD_HANDLER_REG("/cgi-bin/get_jffs2_image.cgi", cgi_get_jffs2_image);
#endif
