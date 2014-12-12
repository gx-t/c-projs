#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#ifndef NO_UUID_LIB
#include <uuid/uuid.h>
#endif //NO_UUID_LIB

#define PAGE_SIZE			0x1000
#define MAX_CMD_LENGTH			0x100
#define MIN_PASSWORD_LENGTH		5
#define UUID_SIZE			36
#define USER_INFO_FIELD_COUNT		5
#define HEART_BEAT_PERIOD		5
static const char users_root[]		= "users/";
static const char user_dir_inbox[]	= "inbox/";
static const char passwd_file_name[]	= ".passwd";
static const char user_info_file_name[]	= ".info";
static const char pid_file_name[]	= ".pid";

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
		while(*pp == '\n' && pp > buff) *pp-- = 0, length --;
	}
	return length;
}

static size_t str_read_line_ff(char* buff, size_t buff_size, FILE* ff)
{
	size_t length = 0;
	char* pp = buff;
	*buff = 0;
	if(!fgets(buff, buff_size, ff)) return 0;
	while(*pp) pp++;
	length = (size_t)(pp - buff);
	if(length) {
		pp --;
		while(*pp == '\n' && pp > buff) *pp-- = 0, length --;
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

static int check_client_password() {
	char passwd1[PAGE_SIZE];
	char passwd2[PAGE_SIZE];
	size_t passwd_len = str_read_line(passwd1, sizeof(passwd1));
	if(passwd_len < MIN_PASSWORD_LENGTH) return 1;
	FILE* ff = fopen(passwd_file_name, "r");
	if(!ff) {
		perror(passwd_file_name);
		return 2;
	}
	*passwd2 = 0;
	str_read_line_ff(passwd2, sizeof(passwd2), ff);
	fclose(ff);
	return  !!strcmp(passwd1, passwd2);
}

static int read_client_uuid_and_go_home()
{
	char uuid[UUID_SIZE + 2];
	if(UUID_SIZE != str_read_line(uuid, sizeof(uuid))) return 1;
	if(chdir(users_root) || chdir(uuid)) {
		resp_text_err();
		puts("Rejected. Unknown user");
		return 2;
	}
	return 0;
}

static int register_notification_pid()
{
	pid_t npid = getpid();
	FILE* ff = fopen(pid_file_name, "w");
	if(!ff) {
		perror(pid_file_name);
		return 1;
	}
	fprintf(ff, "%d", npid);
	fclose(ff);
	return 0;
}

static void send_and_delete_msg(const char* file_name)
{
	char buff[PAGE_SIZE];
	FILE* ff = fopen(file_name, "r");
	if(!ff) return;
	size_t ll = fread(buff, 1, PAGE_SIZE, ff);
	fclose(ff);
	unlink(file_name);
	if(ll <= 0) return;
	printf("message %ld\n", ll);
	fwrite(buff, 1, ll, stdout);
}

static void scan_inbox_and_send()
{
	if(!chdir(user_dir_inbox)) {
		struct dirent *de = 0;
		DIR* pd = opendir(".");
		if(pd) {
			while((de = readdir(pd))) {
				if(strstr(de->d_name, "msg") == de->d_name)
					send_and_delete_msg(de->d_name);
			}
			closedir(pd);
		}
		if(chdir("..")) perror("chdir");
	}
}

static void event_loop()
{
	resp_text_ok();
	scan_inbox_and_send();
}

static void remove_notification_pid()
{
	unlink(pid_file_name);
}

static void cmd_login()
{
	if(read_client_uuid_and_go_home()) return;
	if(check_client_password()) return;
	if(register_notification_pid()) return;
	event_loop();
	remove_notification_pid();
}

static void read_and_save_message()
{
	char buff[PAGE_SIZE];
	char file_name[UUID_SIZE + 5] = "msg-";
	gen_uuid(file_name + 4);
	FILE* ff = fopen(file_name, "w");
	if(!ff) {
		perror(file_name);
		return;
	}
	while(fgets(buff, sizeof(buff), stdin)) {
		fprintf(ff, "%s", buff);
	}
	fclose(ff);
}

static void put_message_to_inbox()
{
	char uuid[UUID_SIZE + 2];
	if(UUID_SIZE != str_read_line(uuid, sizeof(uuid))) return;
	if(chdir("..") || chdir(uuid) || chdir(user_dir_inbox)) {
		resp_text_err();
		puts("Invalid recepient user ID");
		return;
	}
	read_and_save_message();
	resp_text_ok();
}

static void cmd_message()
{
	if(read_client_uuid_and_go_home()) return;
	if(check_client_password()) return;
	put_message_to_inbox();
}

static void* dispatch_cmd()
{
	char cmd_buff[MAX_CMD_LENGTH];
	if(!fgets(cmd_buff, sizeof(cmd_buff), stdin)) return cmd_unknown;
	if(!strcmp(cmd_buff, "echo-line\n")) return cmd_echo_line;
	if(!strcmp(cmd_buff, "register\n")) return cmd_register;
	if(!strcmp(cmd_buff, "login\n")) return cmd_login;
	if(!strcmp(cmd_buff, "message\n")) return cmd_message;
	return cmd_unknown;
}

int main(int argc, char* argv[])
{
	void (*cmd_handler)() = dispatch_cmd();
	cmd_handler();
	return 0;
}



