#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <wait.h>

#define SHM_KEY 12345
#define MSG_KEY 54321

typedef struct {
    unsigned int seconds;
    unsigned int nanoseconds;
} Clock;

typedef struct {
    long msg_type;
    char message[100];
} Message;

int main() {
    int shm_id;
    Clock *system_clock;
    pid_t child_pid;

    // Create shared memory for system clock
    shm_id = shmget(SHM_KEY, sizeof(Clock), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("Shared memory creation failed");
        exit(1);
    }
    system_clock = (Clock *)shmat(shm_id, NULL, 0);
    if ((void *)system_clock == (void *)-1) {
        perror("Shared memory attach failed");
        exit(1);
    }
    system_clock->seconds = 0;
    system_clock->nanoseconds = 0;

    int msg_id = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msg_id == -1) {
        perror("Message queue creation failed");
        exit(1);
    }

    if ((child_pid = fork()) == 0) {
        // Child
        char msg_id_str[15];
        sprintf(msg_id_str, "%d", msg_id);
        
        char *args[] = {"./user_process", msg_id_str, NULL};
        execv("./user_process", args);
        
        perror("Failed to exec user_process");
        exit(1);
    } else if (child_pid > 0) {
        // Parent
        Message msg;
        msgrcv(msg_id, &msg, sizeof(msg.message), 1, 0); // Read message of type 1
        printf("Received message from child: %s\n", msg.message);
        
        // Wait for child to terminate
        wait(NULL);
    } else {
        perror("Fork failed");
        exit(1);
    }

    // Clean up
    shmdt(system_clock);
    shmctl(shm_id, IPC_RMID, NULL);
    msgctl(msg_id, IPC_RMID, NULL);
    return 0;
}

