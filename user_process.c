#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    int msg_id;
    Clock *system_clock;

    // Get the message queue ID from the command line arguments
    if (argc < 2) {
        fprintf(stderr, "user_process: Missing message queue ID\n");
        exit(1);
    }
    msg_id = atoi(argv[1]);

    // Attach to the shared memory
    int shm_id = shmget(SHM_KEY, sizeof(Clock), 0666);
    if (shm_id == -1) {
        perror("user_process: Shared memory retrieval failed");
        exit(1);
    }
    system_clock = (Clock *)shmat(shm_id, NULL, 0);
    if ((void *)system_clock == (void *)-1) {
        perror("user_process: Shared memory attach failed");
        exit(1);
    }

    // For demo purposes: let's assume user process will update the system clock and then send a message to oss
    system_clock->seconds += 1;
    system_clock->nanoseconds += 500;

    Message msg;
    msg.msg_type = 1;
    snprintf(msg.message, sizeof(msg.message), "User process updated clock to: %u seconds, %u nanoseconds", system_clock->seconds, system_clock->nanoseconds);

    // Send the message
    if (msgsnd(msg_id, &msg, sizeof(msg.message), 0) == -1) {
        perror("user_process: Message send failed");
        shmdt(system_clock);
        exit(1);
    }

    // Detach from shared memory
    shmdt(system_clock);
    return 0;
}

