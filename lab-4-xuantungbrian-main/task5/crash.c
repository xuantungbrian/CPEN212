#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 1024
#define MAXJOBS 1024

char **environ;
void convert_int_to_string(int integer, char *string);

typedef struct {
	int process_ID[20000];
	char * status[20000];
	char command[20000][20000];
	int foreground_pid;
	int count;
} job;

//declaring global variable
job job_list;

void handle_sigchld(int sig) {
	int i = 0;
	int count = 0;
	int s = 0;
	char mess[1025] = "";

	while (1) {
		count = 0;
		i = waitpid(-1, &s, WNOHANG | WUNTRACED);
		if (i == -1) { break; }

		//find process
		while (1) {
			if (count >= job_list.count) { break; }
			if (i == job_list.process_ID[count]) {
				break;
			}
			count++;
		}
		if (count >= job_list.count) { break; }

		//categorize and change status
		if (WIFEXITED(s)) {
			job_list.status[count] = "killed";
			if (i == job_list.foreground_pid) { job_list.foreground_pid = 0; }

		}
		else if (WIFSTOPPED(s)) {
			job_list.status[count] = "suspended";
			if (i == job_list.foreground_pid) { job_list.foreground_pid = 0; }

			char foreground_count[1025];
			char foreground_pid[1025];
			convert_int_to_string(count + 1, foreground_count);
			convert_int_to_string(job_list.process_ID[count], foreground_pid);

			strcat(mess, "[");
			strcat(mess, foreground_count);
			strcat(mess, "]");
			strcat(mess, " ");
			strcat(mess, "(");
			strcat(mess, foreground_pid);
			strcat(mess, ")");
			strcat(mess, "  ");
			strcat(mess, "suspended");
			strcat(mess, "  ");
			strcat(mess, job_list.command[count]);
			strcat(mess, "\n");
			write(1, mess, strlen(mess));
		}
		else {
			job_list.status[count] = "killed";
			if (i == job_list.foreground_pid) { job_list.foreground_pid = 0; }

			char foreground_count[1025];
			char foreground_pid[1025];
			convert_int_to_string(count + 1, foreground_count);
			convert_int_to_string(job_list.process_ID[count], foreground_pid);

			strcat(mess, "[");
			strcat(mess, foreground_count);
			strcat(mess, "]");
			strcat(mess, " ");
			strcat(mess, "(");
			strcat(mess, foreground_pid);
			strcat(mess, ")");
			strcat(mess, "  ");
			strcat(mess, "killed");
			strcat(mess, "  ");
			strcat(mess, job_list.command[count]);
			strcat(mess, "\n");
			write(1, mess, strlen(mess));
		}		
	}
}

void handle_sigtstp(int sig) {
	int count = 0;
	char mess[1025] = "";
	if (job_list.foreground_pid == 0) { return; }
	while (1) {
		if (job_list.foreground_pid == job_list.process_ID[count]) {
			break;
		}
		count++;
		if (count >= 20000) { return; }
		if (job_list.process_ID[count] == 0) { return; }
	}
	
	//send sigtstp
	kill(job_list.foreground_pid, SIGTSTP);
}

void handle_sigint(int sig) {
	int count = 0;
	char mess[1025] = "";

	if (job_list.foreground_pid == 0) { return; }
	while (1) {
		if (job_list.foreground_pid == job_list.process_ID[count]) {
			break;
		}
		count++;
		if (count >= 20000) { return; }
		if (job_list.process_ID[count] == 0) { return; }
	}

	char foreground_count[1025];
	char foreground_pid[1025];
	convert_int_to_string(count + 1, foreground_count);
	convert_int_to_string(job_list.process_ID[count], foreground_pid);

	kill(job_list.foreground_pid, SIGINT);
}

void handle_sigquit(int sig) {
	int count = 0;
	char mess[1025] = "";

	if (job_list.foreground_pid == 0) { exit(0); }
	while (1) {
		if (job_list.foreground_pid == job_list.process_ID[count]) {
			break;
		}
		count++;
		if (count >= 20000) { return; }
		if (job_list.process_ID[count] == 0) { return; }
	}

	kill(job_list.foreground_pid, SIGQUIT);
}

void install_signal_handlers() {
	//install sigchild handler
	struct sigaction act;
	act.sa_handler = handle_sigchld;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGCHLD);
	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGQUIT);
	sigaddset(&act.sa_mask, SIGTSTP);
	sigaction(SIGCHLD, &act, NULL);

	//install sigtstp handler (Ctrl+Z)
	struct sigaction act1;
	act1.sa_handler = handle_sigtstp;
	act1.sa_flags = SA_RESTART;
	sigemptyset(&act1.sa_mask);
	sigaction(SIGTSTP, &act1, NULL);

	//install sigint handler (Ctrl+C)
	struct sigaction act2;
	act2.sa_handler = handle_sigint;
	act2.sa_flags = SA_RESTART;
	sigemptyset(&act2.sa_mask);
	sigaction(SIGINT, &act2, NULL);

	//install sigquit handler (Ctrl+\)
	struct sigaction act3;
	act3.sa_handler = handle_sigquit;
	act3.sa_flags = SA_RESTART;
	sigemptyset(&act3.sa_mask);
	sigaction(SIGQUIT, &act3, NULL);
}

void spawn(const char **toks, bool bg) { // need fix if bg is false, check with tab
	int i = 0;
	int s = 0;

	//fork, mask, see if its the child to do execvp, check something about the background
	if (bg) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGQUIT);
		sigaddset(&mask, SIGTSTP);
		sigprocmask(SIG_BLOCK, (const sigset_t *)&mask, NULL);
		pid_t p1 = fork();
		if (p1 < 0) { return; } //do we need to print anything in this?
		else if (p1 == 0) { //child's part
			sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);
			setpgid(0, 0);
			if (execvp(toks[0], (char **)toks) == -1) {
				fprintf(stderr, "ERROR: cannot run %s\n", toks[0]);
				kill(getpid(), SIGKILL);
			}
		}
		else { //parent's part
			job_list.count++;
			job_list.process_ID[job_list.count-1] = p1;
			job_list.status[job_list.count-1] = "running";
			strcpy(job_list.command[job_list.count-1], toks[0]);
			printf("[%d] (%d)  %s\n", job_list.count, job_list.process_ID[job_list.count-1], job_list.command[job_list.count-1]);
			sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);
		}
	}
	else { //foreground
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGQUIT);
		sigaddset(&mask, SIGTSTP);
		sigprocmask(SIG_BLOCK, (const sigset_t *)&mask, NULL);
		pid_t p1 = fork();
		if (p1 < 0) { return; } //do we need to print anything in this?
		else if (p1 == 0) { //child's part
			sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);
			setpgid(0, 0);
			if (execvp(toks[0], (char **)toks) == -1) {
				fprintf(stderr, "ERROR: cannot run %s\n", toks[0]);
				kill(getpid(), SIGKILL);
			}
		}
		else { //parent's part
			job_list.count++;
			job_list.process_ID[job_list.count - 1] = p1;
			job_list.status[job_list.count - 1] = "running"; // need change to suspended or sth
			strcpy(job_list.command[job_list.count - 1], toks[0]);
			job_list.foreground_pid = p1;
			sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);
			while (1) {
				if (strcmp(job_list.status[job_list.count - 1], "killed") == 0 || strcmp(job_list.status[job_list.count - 1], "suspended") == 0) { 
					break; 
				}
				sleep(0.01);
			}
		}
	}
}

void cmd_jobs(const char **toks) { //check if the process is killed
	int i = 0;
	if (toks[1] != NULL) {
		fprintf(stderr, "ERROR: jobs takes no arguments\n");
		return;
	}
	else {
		while (i < job_list.count ) {
			if (strcmp(job_list.status[i],"killed") != 0) {
				printf("[%d] (%d)  %s  %s\n", i+1, job_list.process_ID[i], job_list.status[i], job_list.command[i]);
			}
			i++;
		}
	}
}

void cmd_fg(const char **toks) {
	int pid = 0;
	int job = 0;
	int i = 0;
	char med[1024];
	char * ptr;

	//Check number of arguments
	if (toks[2] != NULL || toks[1] == NULL) {
		fprintf(stderr, "ERROR: fg takes exactly one argument\n");
		return;
	}

	//Check job or process
	strcpy(med, toks[1]);
	if (toks[1][0] == '%') {
		job = 1;
		med[0] = '0';
	}

	if (strcmp(toks[1], "%") == 0) {
		fprintf(stderr, "ERROR: bad argument for fg: %s\n", toks[1]);
		return;
	}

	//Take process ID
	if (job == 0) {
		pid = strtol(med, &ptr, 10);
	}
	else {
		i = strtol(med, &ptr, 10);
	}
	if (strcmp(ptr, "") != 0) {
		fprintf(stderr, "ERROR: bad argument for fg: %s\n", toks[1]);
		return;
	}

	if (job == 1 && i >= 1) {
		pid = job_list.process_ID[i - 1];
	}

	//Check if the process is in job and if its killed
	if (job == 1) {
		if (i > job_list.count || i < 1 || pid == 0 || strcmp(job_list.status[i - 1], "killed") == 0) {
			fprintf(stderr, "ERROR: no job %%%d\n", i);
			return;
		}
		i--;
	}
	else {
		while (1) {
			if (i >= job_list.count) {
				fprintf(stderr, "ERROR: no PID %d\n", pid);
				return;
			}
			if (pid == job_list.process_ID[i]) {
				if (strcmp(job_list.status[i], "killed") != 0) {
					break;
				}
				else {
					fprintf(stderr, "ERROR: no PID %d\n", pid);
					return;
				}
			}
			i++;
		}
	}

	//Resume the process
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_BLOCK, (const sigset_t *)&mask, NULL);
	if (strcmp(job_list.status[i], "suspended") == 0) {
		kill(pid, SIGCONT);
		job_list.status[i] = "running";
	}

	//Make the process foreground
	job_list.foreground_pid = pid;
	sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);

	while (1) {
		if (strcmp(job_list.status[i], "killed") == 0 || strcmp(job_list.status[i], "suspended") == 0) {
			break;
		}
		sleep(0.01);
	}
}

void cmd_bg(const char **toks) { // need change with fg
	int pid =0;
	int job = 0;
	int i = 0;
	char med[1024];
	char * ptr;

	//Check number of arguments
	if (toks[2] != NULL || toks[1] == NULL) {
		fprintf(stderr, "ERROR: bg takes exactly one argument\n");
		return;
	}

	//Check job or process
	strcpy(med, toks[1]);
	if (toks[1][0] == '%') {
		job = 1;
		med[0] = '0';
	}

	if (strcmp(toks[1], "%") == 0) {
		fprintf(stderr, "ERROR: bad argument for bg: %s\n", toks[1]);
		return;
	}

	//Take process ID
	if (job == 0) {
		pid = strtol(med, &ptr, 10);
	}
	else {
		i = strtol(med, &ptr, 10);
	}
	if (strcmp(ptr, "") != 0) {
		fprintf(stderr, "ERROR: bad argument for bg: %s\n", toks[1]);
		return;
	}

	if (job == 1 && i >= 1) {
		pid = job_list.process_ID[i - 1];
	}

	//Check if the process is in job and if its killed
	if (job == 1) {
		if (i > job_list.count || i < 1 || pid == 0 || strcmp(job_list.status[i - 1], "killed") == 0) {
			fprintf(stderr, "ERROR: no job %%%d\n", i);
			return;
		}
		i--;
	}
	else {
		while (1) {
			if (i >= job_list.count) {
				fprintf(stderr, "ERROR: no PID %d\n", pid);
				return;
			}
			if (pid == job_list.process_ID[i]) {
				if (strcmp(job_list.status[i], "killed") != 0) {
					break;
				}
				else {
					fprintf(stderr, "ERROR: no PID %d\n", pid);
					return;
				}
			}
			i++;
		}
	}

	printf("[%d] (%d)  %s\n", i+1, job_list.process_ID[i], job_list.command[i]);

	//Resume the process
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_BLOCK, (const sigset_t *)&mask, NULL);
	if (strcmp(job_list.status[i], "suspended") == 0) {
		kill(pid, SIGCONT);
		job_list.status[i] = "running";
	}

	//Check if the process if background
	if (pid != job_list.foreground_pid) {
		return;
	}
	else {
		job_list.foreground_pid = 0;
	}

	sigprocmask(SIG_UNBLOCK, (const sigset_t *)&mask, NULL);
}

void cmd_slay(const char **toks) {
	int pid = 0;
	int job = 0;
	int i = 0;
	char med[1024];
	char * ptr;

	//Check number of arguments
	if (toks[2] != NULL || toks[1] == NULL) {
		fprintf(stderr, "ERROR: slay takes exactly one argument\n");
		return;
	}

	//Check job or process
	strcpy(med, toks[1]);
	if (toks[1][0] == '%') {
		job = 1;
		med[0] = '0';
	}

	if (strcmp(toks[1], "%") == 0) {
		fprintf(stderr, "ERROR: bad argument for slay: %s\n", toks[1]);
		return;
	}

	//Take process ID
	if (job == 0) {
		pid = strtol(med, &ptr, 10);
	}
	else {
		i = strtol(med, &ptr, 10);
	}
	if (strcmp(ptr, "") != 0) {
		fprintf(stderr, "ERROR: bad argument for slay: %s\n", toks[1]);
		return;
	}

	if (job == 1 && i >= 1) {
		pid = job_list.process_ID[i - 1];
	}

	//Check if the process is in job and if its killed
	if (job == 1) {
		if (i > job_list.count || i < 1 || pid == 0 || strcmp(job_list.status[i - 1], "killed") == 0) {
			fprintf(stderr, "ERROR: no job %%%d\n", i);
			return;
		}
		i--;
	}
	else {
		while (1) {
			if (i >= job_list.count) {
				fprintf(stderr, "ERROR: no PID %d\n", pid);
				return;
			}
			if (pid == job_list.process_ID[i]) {
				if (strcmp(job_list.status[i], "killed") != 0) {
					break;
				}
				else {
					fprintf(stderr, "ERROR: no PID %d\n", pid);
					return;
				}
			}
			i++;
		}
	}

	kill(pid, SIGKILL);
}

void cmd_quit(const char **toks) {
	if (toks[1] != NULL) {
		fprintf(stderr, "ERROR: quit takes no arguments\n");
	}
	else {
		exit(0);
	}
}

void eval(const char **toks, bool bg) { // bg is true iff command ended with &
	assert(toks);
	if (*toks == NULL) return;
	if (strcmp(toks[0], "quit") == 0) {
		cmd_quit(toks);
	}
	else if (strcmp(toks[0], "fg") == 0) {
		cmd_fg(toks);
	}
	else if (strcmp(toks[0], "bg") == 0) {
		cmd_bg(toks);
	}
	else if (strcmp(toks[0], "jobs") == 0) {
		cmd_jobs(toks);
	}
	else if (strcmp(toks[0], "slay") == 0) {
		cmd_slay(toks);
	}
	else {
		spawn(toks, bg);
	}
}

// you don't need to touch this unless you are submitting the bonus task
void parse_and_eval(char *s) {
	assert(s);
	const char *toks[MAXLINE + 1];

	while (*s != '\0') {
		bool end = false;
		bool bg = false;
		int t = 0;

		while (*s != '\0' && !end) {
			while (*s == ' ' || *s == '\t' || *s == '\n') ++s;
			if (*s != '&' && *s != ';' && *s != '\0') toks[t++] = s;
			while (strchr("&; \t\n", *s) == NULL) ++s;
			switch (*s) {
			case '&':
				bg = true;
				end = true;
				break;
			case ';':
				end = true;
				break;
			}
			if (*s) *s++ = '\0'; // the changed part from patch
		}
		toks[t] = NULL;
		eval(toks, bg);
	}
}

// you don't need to touch this unless you are submitting the bonus task
void prompt() {
	printf("crash> ");
	fflush(stdout);
}

// you don't need to touch this unless you are submitting the bonus task
int repl() {
	char *line = NULL;
	size_t len = 0;
	while (prompt(), getline(&line, &len, stdin) != -1) {
		parse_and_eval(line);
	}

	if (line != NULL) free(line);
	if (ferror(stdin)) {
		perror("ERROR");
		return 1;
	}
	return 0;
}

// you don't need to touch this unless you want to add debugging options
int main(int argc, char **argv) {
	install_signal_handlers();
	return repl();
}

//function converting int to string
void convert_int_to_string(int integer, char *string) {
	int base = 10;
	char *ptr = string, tmp_char;
	int tmp_value;
	int value = integer;
	char * alphabet = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

	while (1) {
		tmp_value = value;
		value /= base;
		*string++ = alphabet[35 + (tmp_value - value * base)];
		if (value == 0) { break; }
	}

	*string-- = '\0';

	while (ptr < string) {
		tmp_char = *string;
		*string-- = *ptr;
		*ptr++ = tmp_char;
	}
}
