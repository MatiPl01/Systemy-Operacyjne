#include <stdio.h>
#include <string.h>


#define ORDER_BY_DATE "date"
#define ORDER_BY_SENDER "sender"

int print_emails_ordered_by(char* order_by);
int send_email(char* recipient_email, char* title, char* content);
void print_centered(char* text, int width, char fill_char);
void print_results(FILE *f_ptr);


int main(int argc, char* argv[]) {
    switch (argc) {
        case 2:
            if (print_emails_ordered_by(argv[1]) == -1) return 1;
            return 0;
        case 4:
            if (send_email(argv[1], argv[2], argv[3]) == -1) return 1;
            return 0;
        default:
            fprintf(stderr, "Wrong number of arguments.\n");
            return 1;
    }
}


int print_emails_ordered_by(char* order_by) {
    FILE* f_ptr;
    char* cmd;

    if (strcmp(order_by, ORDER_BY_DATE) == 0) {
        cmd = "echo | mail -f | tac";
    } else if (strcmp(order_by, ORDER_BY_SENDER) == 0) {
        cmd = "echo | mail -f | sort -k 2";
    } else {
        fprintf(stderr, "Unrecognized order by condition.\n");
        return -1;
    }

    if (!(f_ptr = popen(cmd, "r"))) {
        perror("Unable to open a pipe.\n");
        return -1;
    }

    char buff[32];
    sprintf(buff, " Mails ordered by: %s ", order_by);
    print_centered(buff, 50, '=');
    print_results(f_ptr);
    return 0;
}

int send_email(char* recipient_email, char* title, char* content) {
    char cmd[strlen(recipient_email) + strlen(title) + strlen(content) + 17];
    sprintf(cmd, "echo %s | mail -s %s %s", content, title, recipient_email);

    if (!popen(cmd, "r")) {
        perror("Unable to open a pipe.\n");
        return -1;
    }

    print_centered(" Email was successfully sent ", 50, '=');
    puts("Email details:");
    printf("Recipient: %s\n", recipient_email);
    printf("Title:     %s\n", title);
    printf("Content: \n%s\n", content);

    return 0;
}

void print_centered(char* text, int width, char fill_char) {
    size_t length = strlen(text);

    if (length >= width) {
        puts(text);
        return;
    }

    size_t l_just = (width - length) / 2;
    for (int i = 0; i < l_just; i++) printf("%c", fill_char);
    printf("%s", text);
    for (int i = 0; i < width - length - l_just; i++) printf("%c", fill_char);
    printf("\n");
}

void print_results(FILE *f_ptr) {
    char* line;
    while (getline(&line, NULL, f_ptr) != -1) {
        puts(line);
    }
}
