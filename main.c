#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUD_FILE "stud_db.txt"
#define LOGIN_FILE "auth_db.txt"

typedef struct {
    int roll;
    char fullName[60];
    int academicYear;
    char sec;
    float score;
} Record;

void inputString(char *msg, char *buf, int n) {
    printf("%s", msg);
    fgets(buf, n, stdin);
    if (buf[0] == '\n') fgets(buf, n, stdin);
    buf[strcspn(buf, "\n")] = '\0';
}

int inputInt(char *msg) {
    char temp[40];
    int val;
    while (1) {
        printf("%s", msg);
        fgets(temp, sizeof(temp), stdin);
        if (sscanf(temp, "%d", &val) == 1) return val;
        printf("Invalid input! Enter a number.\n");
    }
}

float inputFloat(char *msg, float min, float max) {
    char temp[40];
    float f;
    while (1) {
        printf("%s", msg);
        fgets(temp, sizeof(temp), stdin);
        if (sscanf(temp, "%f", &f) == 1 && f >= min && f <= max) return f;
        printf("Enter a valid value between %.2f and %.2f.\n", min, max);
    }
}

int rollExists(int r) {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) return 0;

    Record rec;
    while (fscanf(fp, "%d %s %d %c %f", 
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {
        if (rec.roll == r) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

int verifyUser(char *u, char *p, char *role, int *idOut) {
    FILE *fp = fopen(LOGIN_FILE, "r");
    if (!fp) return 0;

    char usr[50], pass[50], r;
    int num;
    while (fscanf(fp, "%s %s %c %d", usr, pass, &r, &num) == 4) {
        if (!strcmp(usr, u) && !strcmp(pass, p)) {
            *role = r;
            *idOut = num;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void addRecord() {
    Record rec;
    FILE *fp;

    printf("\n---- Add New Student ----\n");

    while (1) {
        rec.roll = inputInt("Enter Roll Number: ");
        if (!rollExists(rec.roll)) break;
        printf("Roll number already in use. Try again.\n");
    }

    inputString("Enter Name (one word): ", rec.fullName, 60);
    if (strchr(rec.fullName, ' ')) {
        printf("Spaces not allowed in name.\n");
        return;
    }

    rec.academicYear = inputInt("Enter Year: ");

    char s[10];
    while (1) {
        inputString("Enter Section (A-Z): ", s, 10);
        if (strlen(s) == 1 && isalpha(s[0])) {
            rec.sec = toupper(s[0]);
            break;
        }
        printf("Invalid section.\n");
    }

    rec.score = inputFloat("Enter CGPA: ", 0, 10);

    fp = fopen(STUD_FILE, "a");
    fprintf(fp, "%d %s %d %c %.2f\n", rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
    fclose(fp);

    char un[50], pw[50];
    sprintf(un, "student%d", rec.roll);
    sprintf(pw, "pwd%d@", rec.roll);

    fp = fopen(LOGIN_FILE, "a");
    fprintf(fp, "%s %s S %d\n", un, pw, rec.roll);
    fclose(fp);

    printf("\nStudent added successfully!\n");
    printf("Assigned Login -> Username: %s | Password: %s\n", un, pw);
}

void displayAll() {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("No data available.\n");
        return;
    }

    Record rec;
    printf("\n---- Student List ----\n");

    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {

        printf("\nRoll: %d\nName: %s\nYear: %d\nSection: %c\nCGPA: %.2f\n",
               rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
    }
    fclose(fp);
}

void searchRecord() {
    int r = inputInt("Enter Roll to search: ");

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) { printf("No data.\n"); return; }

    Record rec;
    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {

        if (rec.roll == r) {
            printf("\nFOUND!\nRoll: %d\nName: %s\nYear: %d\nSection: %c\nCGPA: %.2f\n",
                   rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
            fclose(fp);
            return;
        }
    }

    printf("Record not found.\n");
    fclose(fp);
}

void updateRecord() {
    int r = inputInt("Enter Roll to update: ");

    FILE *fp = fopen(STUD_FILE, "r");
    FILE *temp = fopen("tempfile.txt", "w");

    if (!fp) { printf("No data.\n"); return; }

    Record rec;
    int ok = 0;

    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {

        if (rec.roll == r) {
            ok = 1;
            printf("1. Change Year\n2. Change Section\n3. Change CGPA\n");
            int c = inputInt("Select: ");

            if (c == 1) rec.academicYear = inputInt("New Year: ");
            else if (c == 2) {
                char t[10];
                while (1) {
                    inputString("New Section: ", t, 10);
                    if (strlen(t) == 1 && isalpha(t[0])) {
                        rec.sec = toupper(t[0]);
                        break;
                    }
                }
            }
            else if (c == 3) rec.score = inputFloat("New CGPA: ", 0, 10);
        }
        fprintf(temp, "%d %s %d %c %.2f\n",
                rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
    }

    fclose(fp);
    fclose(temp);

    remove(STUD_FILE);
    rename("tempfile.txt", STUD_FILE);

    if (ok) printf("Record updated.\n");
    else printf("Roll not found.\n");
}

void modifyCGPA() {
    int r = inputInt("Enter Roll: ");

    FILE *fp = fopen(STUD_FILE, "r");
    FILE *tmp = fopen("tmpfile.txt", "w");
    if (!fp) { printf("No data.\n"); return; }

    Record rec;
    int found = 0;

    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {

        if (rec.roll == r) {
            found = 1;
            printf("Old CGPA: %.2f\n", rec.score);
            rec.score = inputFloat("New CGPA: ", 0, 10);
        }
        fprintf(tmp, "%d %s %d %c %.2f\n", rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
    }

    fclose(fp); fclose(tmp);
    remove(STUD_FILE);
    rename("tmpfile.txt", STUD_FILE);

    if (found) printf("CGPA updated.\n");
    else printf("Roll not found.\n");
}

void removeRecord() {
    int r = inputInt("Enter Roll to delete: ");

    FILE *fp = fopen(STUD_FILE, "r");
    FILE *tmp = fopen("tfile.txt", "w");

    if (!fp) { printf("No data.\n"); return; }

    Record rec;
    int del = 0;

    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {

        if (rec.roll == r) { del = 1; continue; }
        fprintf(tmp, "%d %s %d %c %.2f\n",
                rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
    }

    fclose(fp); fclose(tmp);
    remove(STUD_FILE);
    rename("tfile.txt", STUD_FILE);

    FILE *cf = fopen(LOGIN_FILE, "r");
    FILE *ct = fopen("logtmp.txt", "w");
    char u[50], p[50], role;
    int id;

    if (cf) {
        while (fscanf(cf, "%s %s %c %d", u, p, &role, &id) == 4) {
            if (id == r) continue;
            fprintf(ct, "%s %s %c %d\n", u, p, role, id);
        }
        fclose(cf); fclose(ct);
        remove(LOGIN_FILE);
        rename("logtmp.txt", LOGIN_FILE);
    }

    if (del) printf("Record removed.\n");
    else printf("Roll not found.\n");
}

void createTeacher() {
    FILE *fp = fopen(LOGIN_FILE, "r");
    int cnt = 0;
    if (fp) {
        char u[50], p[50], r;
        int id;
        while (fscanf(fp, "%s %s %c %d", u, p, &r, &id) == 4)
            if (r == 'T') cnt++;
        fclose(fp);
    }

    char un[50], pw[50];
    sprintf(un, "tchr%d", cnt + 1);
    sprintf(pw, "teach%d#", cnt + 1);

    fp = fopen(LOGIN_FILE, "a");
    fprintf(fp, "%s %s T -1\n", un, pw);
    fclose(fp);

    printf("Teacher account created.\nLogin: %s | %s\n", un, pw);
}

void adminPanel() {
    while (1) {
        printf("\n--- Admin Panel ---\n");
        printf("1. Add Student\n2. View Students\n3. Search\n4. Update\n5. Delete\n6. Add Teacher\n7. Back\n");

        int c = inputInt("Choose: ");
        if (c == 1) addRecord();
        else if (c == 2) displayAll();
        else if (c == 3) searchRecord();
        else if (c == 4) updateRecord();
        else if (c == 5) removeRecord();
        else if (c == 6) createTeacher();
        else if (c == 7) return;
    }
}

void teacherPanel() {
    while (1) {
        printf("\n--- Teacher Panel ---\n");
        printf("1. View Students\n2. Search\n3. Modify CGPA\n4. Logout\n");

        int c = inputInt("Option: ");
        if (c == 1) displayAll();
        else if (c == 2) searchRecord();
        else if (c == 3) modifyCGPA();
        else if (c == 4) return;
    }
}

void studentPanel(int id) {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) { printf("No data.\n"); return; }

    Record rec;
    printf("\n--- Student Details ---\n");

    while (fscanf(fp, "%d %s %d %c %f",
                  &rec.roll, rec.fullName, &rec.academicYear, &rec.sec, &rec.score) == 5) {
        if (rec.roll == id) {
            printf("Roll: %d\nName: %s\nYear: %d\nSection: %c\nCGPA: %.2f\n",
                    rec.roll, rec.fullName, rec.academicYear, rec.sec, rec.score);
            break;
        }
    }
    fclose(fp);
}

int main() {
    char user[50], pass[50];

    while (1) {
        printf("\n==== Main Menu ====\n1. Login\n2. Exit\n");
        int ch = inputInt("Choose: ");
        if (ch == 2) break;

        inputString("Username: ", user, 50);
        inputString("Password: ", pass, 50);

        char role;
        int id;

        if (verifyUser(user, pass, &role, &id)) {
            if (role == 'A') adminPanel();
            else if (role == 'T') teacherPanel();
            else if (role == 'S') studentPanel(id);
        } else {
            printf("Invalid login!\n");
        }
    }

    return 0;
}
