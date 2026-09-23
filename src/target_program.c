#include <stdio.h>
#include <unistd.h>

int main(void) {
    // Get the current process ID
    pid_t process_id = getpid();

    // Print the Process ID
    printf("Current Process ID: %d\n", process_id);
    int health = 100;

    do {
    printf("\nHealth value: %d\nHealth memory address: %p\n", health, &health);
    
    printf("Enter new health value:");
    scanf("%d", &health);
    } while (1);
return 0;
}

