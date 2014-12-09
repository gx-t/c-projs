#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef NO_UUID_LIB
#include <uuid/uuid.h>
#endif //NO_UUID_LIB

#define PAGE_SIZE			0x1000
#define MAX_CMD_LENGTH			0x100
#define MIN_PASSWORD_LENGTH		5
#define UUID_SIZE			36
#define USER_INFO_FIELD_COUNT		5
static const char users_root[]		= "users/";
static const char user_dir_inbox[]	= "inbox/";
static const char passwd_file_name[]	= ".passwd";
static const char user_info_file_name[]	= ".info";

static void resp_text_ok()
{
	puts("Content-type: text/plain; charset=UTF-8\n");
	puts("OK");
}

static void resp_text_err()
{
	puts("Content-type: text/plain; charset=UTF-8\n");
	puts("ERR");
}

static void cmd_unknown()
{
	resp_text_err();
	puts("Unknown command");
}

static void cmd_echo_line()
{
	char buff[PAGE_SIZE];
	if(fgets(buff, sizeof(buff), stdin)) {
		resp_text_ok();
		puts(buff);
	}
}

static size_t str_read_line(char* buff, size_t buff_size)
{
	size_t length = 0;
	char* pp = buff;
	*buff = 0;
	if(!fgets(buff, buff_size, stdin)) return 0;
	while(*pp) pp++;
	length = (size_t)(pp - buff);
	if(length) {
		pp --;
		while(*pp == '\n' && pp <= buff) *pp-- = 0, length --;
	}
	return length;
}


static int register_client_password()
{
	char passwd[PAGE_SIZE];
	size_t length = str_read_line(passwd, sizeof(passwd));
	if(length < MIN_PASSWORD_LENGTH) {
		resp_text_err();
		printf("The speocified password is too short. It must be at least %d characters long.\n", MIN_PASSWORD_LENGTH);
		return 1;
	}
	FILE* ff = fopen(passwd_file_name, "w");
	if(!ff) {
		perror(passwd_file_name);
		return 1;
	}
	fprintf(ff, "%s", passwd);
	fclose(ff);
	return 0;
}


static void gen_uuid(char* buff)
{
#ifdef NO_UUID_LIB
	FILE* pf = popen("uuidgen", "r");
	if(!pf) {
		perror("uuidgen");
		return;
	}
	fread(buff, UUID_SIZE, 1, pf);
	pclose(pf);
	buff[UUID_SIZE] = 0;
#else
	uuid_t uu;
	uuid_generate_time(uu);
	uuid_unparse(uu, buff);
#endif //NO_UUID_LIB
}

static char* db_create_enter_new_user_dir(char* buff)
{
	strcpy(buff, users_root);
	gen_uuid(buff + sizeof(users_root) - 1);
	if(mkdir(buff, 0700) || chdir(buff)) perror(buff);
	return buff + sizeof(users_root) - 1;
}

static void register_client_user_info()
{
	char buff[PAGE_SIZE];
	int count = 0;
	FILE* ff = fopen(user_info_file_name, "w");
	if(!ff) {
		perror(user_info_file_name);
		return;
	}
	while(fgets(buff, sizeof(buff), stdin) && count < USER_INFO_FIELD_COUNT) fprintf(ff, "%s", buff), count++;
	fclose(ff);
}

static void db_make_user_dirs()
{
	if(mkdir(user_dir_inbox, 0700)) perror(user_dir_inbox);
}

static void cmd_register()
{
	char buff[PAGE_SIZE];
	char* uuid_str = db_create_enter_new_user_dir(buff);
	if(register_client_password()) {
		if(chdir("..") || rmdir(uuid_str)) perror("User home cleanup on invalid password");
		return;
	}
	register_client_user_info();
	db_make_user_dirs();
	resp_text_ok();	
	puts(uuid_str);
}

static void* dispatch_cmd()
{
	char cmd_buff[MAX_CMD_LENGTH];
	if(!fgets(cmd_buff, sizeof(cmd_buff), stdin)) return cmd_unknown;
	if(!strcmp(cmd_buff, "echo-line\n")) return cmd_echo_line;
	if(!strcmp(cmd_buff, "register\n")) return cmd_register;
	return cmd_unknown;
}

int main(int argc, char* argv[])
{
	void (*cmd_handler)() = dispatch_cmd();
	cmd_handler();
	return 0;
}



