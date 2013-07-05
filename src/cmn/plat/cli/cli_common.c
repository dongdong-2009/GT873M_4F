#include "plat_common.h"
#include "cli_common.h"
//#include "aal_ma.h"

#ifdef HAVE_TELNET_CLI
struct cli_command *gpst_cmd_tree = NULL;
struct cli_command g_cli_command[MAX_CLI_COMMAND];
#define MAX_USER_CMD_INIT_HANDLER 16
#define USER_CMD_INIT_HANDLE_DESC_LEN 32

extern void cli_reg_usr_cmd(struct cli_command **cmd_root);
extern void user_register_command_entry(struct cli_command **cmd_root);
extern int cli_show_help(struct cli_def *cli, struct cli_command *c);
typedef struct{
	char desc[USER_CMD_INIT_HANDLE_DESC_LEN];
	USER_CMD_INIT func;
}user_init_handler_t;

user_init_handler_t g_user_cmd_init_handlers[MAX_USER_CMD_INIT_HANDLER];
int registerUserCmdInitHandler(const char * desc, USER_CMD_INIT handler)
{
	int i, ret = CS_ERROR, fblank = -1;

	if(desc && handler)
	{
		for(i=0; i<MAX_USER_CMD_INIT_HANDLER; i++)
		{
			user_init_handler_t * phandle = &g_user_cmd_init_handlers[i];
			if(phandle->func == handler)
			{
				ret = CS_OK;
				break;
			}
			else if(phandle->func == NULL)
			{
				if(fblank == -1)
					fblank = i;
				continue;
			}
			else
				continue;
		}

		if(ret == CS_ERROR && fblank != -1)
		{
			int len = strlen(desc) >= (USER_CMD_INIT_HANDLE_DESC_LEN-1)? (USER_CMD_INIT_HANDLE_DESC_LEN-1):strlen(desc);
			strncpy(g_user_cmd_init_handlers[fblank].desc, desc, len);
			g_user_cmd_init_handlers[fblank].func = handler;
			ret = CS_OK;
		}
	}

	return ret;
}
int cli_int_help(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    cli_error(cli, "\nCommands available:");
    cli_show_help(cli, cli->commands);
    return CLI_OK;
}

int cli_int_history(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int i;

    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    cli_error(cli, "\nCommand history:");
    for (i = 0; i < MAX_HISTORY; i++)
    {
        if (strlen(&cli->history[i][0]))
            cli_error(cli, "%3d. %s", i, cli->history[i]);
    }

    return CLI_OK;
}

int cli_int_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if (cli->privilege == PRIVILEGE_PRIVILEGED)
        return CLI_OK;

    if (!cli->enable_password && !cli->enable_callback)
    {
        /* no password required, set privilege immediately */
        cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
        cli_set_configmode(cli, MODE_EXEC, NULL);
    }
    else
    {
        /* require password entry */
        cli->state = STATE_ENABLE_PASSWORD;
    }

    return CLI_OK;
}

int cli_int_disable(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
    cli_set_configmode(cli, MODE_EXEC, NULL);
    return CLI_OK;
}

int cli_int_quit(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
    cli_set_configmode(cli, MODE_EXEC, NULL);
    return CLI_QUIT;
}


int cli_int_exit(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;
    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    if (cli->mode == MODE_EXEC)
        return cli_int_quit(cli, command, argv, argc);

    if (cli->mode > MODE_CONFIG)
        cli_set_configmode(cli, MODE_CONFIG, NULL);
    else
        cli_set_configmode(cli, MODE_EXEC, NULL);

    cli->service = NULL;
    return CLI_OK;
}

extern void cli_debeg_gwd_cmd(struct cli_command **cmd_root);
extern void cli_switch_gwd_cmd(struct cli_command **cmd_root);
extern void cli_reg_gwd_cmd(struct cli_command **cmd_root);
extern void cli_uart_gwd_cmd(struct cli_command **cmd_root);
struct cli_command *cli_tree_init()
{
    struct cli_command *cmd_root = NULL;
    memset(&g_cli_command[0],0,sizeof(struct cli_command)*MAX_CLI_COMMAND);
    // build-in standard commands
    cli_register_command(&cmd_root, 0, "help",    cli_int_help,       PRIVILEGE_UNPRIVILEGED, MODE_ANY,   "Show available commands");
    cli_register_command(&cmd_root, 0, "history", cli_int_history,    PRIVILEGE_UNPRIVILEGED, MODE_ANY,   "Show a list of previously run commands");
    cli_register_command(&cmd_root, 0, "enable",  cli_int_enable,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC,  "Turn on privileged commands");
    cli_register_command(&cmd_root, 0, "disable", cli_int_disable,    PRIVILEGE_PRIVILEGED,   MODE_EXEC,  "Turn off privileged commands");
    cli_register_command(&cmd_root, 0, "quit",    cli_int_quit,       PRIVILEGE_UNPRIVILEGED, MODE_ANY,   "Disconnect");
    cli_register_command(&cmd_root, 0, "logout",  cli_int_quit,       PRIVILEGE_UNPRIVILEGED, MODE_ANY,   "Disconnect");
    cli_register_command(&cmd_root, 0, "exit",    cli_int_exit,       PRIVILEGE_UNPRIVILEGED, MODE_ANY,   "Exit from current mode");
    // reg demo coammnds
    cli_reg_usr_cmd(&cmd_root);
	cli_switch_gwd_cmd(&cmd_root);
	cli_debeg_gwd_cmd(&cmd_root);
	cli_reg_gwd_cmd(&cmd_root);
    user_register_command_entry(&cmd_root);

	#ifdef HAVE_TERMINAL_SERVER
	cli_uart_gwd_cmd(&cmd_root);
	#endif

    return cmd_root;
}

#endif

