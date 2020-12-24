#include <algorithm>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

pid_t __reactive_pid;
int __reactive_input, __reactive_output;
int __standard_input = -1;
int __standard_output = -1;

bool is_connected = false;

static struct _tag_guard{
	bool is_accepted = false;
	~_tag_guard(){
		if (!is_accepted) std::cout << -1 << "\n";
	}
} guard;

int reactive_start(int argc, const char** argv){
	const char** argv_new = (const char**)malloc(sizeof(char*) * argc);
	for (size_t i = 0; i + 1 < argc; ++i)
		argv_new[i] = argv[i + 1];
	argv_new[argc - 1] = nullptr;

	int pipe_c2p[2], pipe_p2c[2];

	signal(SIGPIPE, SIG_IGN);
	if (pipe(pipe_c2p) < 0 || pipe(pipe_p2c) < 0){
		fprintf(stderr, "pipe: failed to open pipes\n");
		return 1;
	}
	if ((__reactive_pid = fork()) < 0){
		fprintf(stderr, "fork: failed to fork\n");
		return 1;
	}
	if (__reactive_pid == 0){
		close(pipe_p2c[1]); close(pipe_c2p[0]);
		dup2(pipe_p2c[0], 0); dup2(pipe_c2p[1], 1);
		close(pipe_p2c[0]); close(pipe_c2p[1]);
		execv(argv_new[0], (char* const*)argv_new);
	}
	free(argv_new);
	close(pipe_p2c[0]); close(pipe_c2p[1]);
	__reactive_input = pipe_p2c[1];
	__reactive_output = pipe_c2p[0];
	return 0;
}

void reactive_connect(){
	if (is_connected) return;
	if (__standard_input == -1) __standard_input = dup(0);
	if (__standard_output == -1) __standard_output = dup(1);

	dup2(__reactive_input, 1);
	dup2(__reactive_output, 0);

	is_connected = true;
}

void reactive_disconnect(){
	if (!is_connected) return;
	fflush(stdout);
	dup2(__standard_input, 0);
	dup2(__standard_output, 1);

	is_connected = false;
}

void reactive_end(){
	int status;
	close(__reactive_input);
/*	sleep(1);
	if(kill(__reactive_pid, 0) == 0){
		kill(__reactive_pid, SIGKILL);
	}
*/
	waitpid(__reactive_pid, &status, WUNTRACED);
}

void reactive_write(std::string buf){
	write(__reactive_input, buf.c_str(), buf.size());
}

std::string reactive_read(size_t max_len = 100000){
	static char buf[1024]; static int len = 0; std::string result;
	while (result.size() < max_len){
		if (!len){
			len = read(__reactive_output, buf,
				std::min(1000, (int)(max_len - result.size())));
			if (!len) return result;
		}
		char* pos = (char*)memchr(buf, '\n', len);
		if (pos){
			result += std::string(buf, pos - buf + 1);
			memmove(buf, pos + 1, len - (pos + 1 - buf));
			len -= pos - buf + 1;
			return result;
		}
		else{
			result += std::string(buf, len);
			len = 0;
		}
	}
	return result;
}
