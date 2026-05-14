#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "catalog.h"
#include "queue.h"
#include "stack.h"
#include "hash.h"
#include "cart.h"
#include "utils.h"

    // User authentication structures
typedef struct User
{

    char password[50];
    char username[50];
    char display_name[50];  /* Display name for menus */
    char role; /* 'S' = Seller, 'B' = Buyer */
    struct User *next;
} User;

typedef struct UserList
{
    User *head;
} UserList;

typedef struct UserHashNode
{
    char username[50];
    User *user;
    struct UserHashNode *next;
} UserHashNode;

typedef struct UserHashTable
{
    UserHashNode *buckets[TABLE_SIZE];
} UserHashTable;

/* ============================================================
   main.c — Trade Hub System
   Entry point. Manages menus and ties all modules together.

   DATA STRUCTURES USED:
     - Linked List  (catalog.c) — item listings
     - Queue        (queue.c)   — buyer inquiries
     - Stack        (stack.c)   — transaction history
     - Hash Table   (hash.c)    — category index [BONUS]

   ALGORITHMS USED:
     - Linear Search   (catalog.c — search_by_keyword)
     - Binary Search   (catalog.c — binary_search_price)
     - Bubble Sort     (catalog.c — sort_by_price)
     - Selection Sort  (catalog.c — sort_by_name)
   ============================================================ */

    // --- Forward declarations for menu functions ---
void main_menu(Catalog *catalog, InquiryQueue *q,
               TxStack *stack, HashTable *ht, ShoppingCart *cart, UserList *users, UserHashTable *user_ht, User *user);
void seller_menu(Catalog *catalog, InquiryQueue *q,
                 TxStack *stack, HashTable *ht, UserList *users, UserHashTable *user_ht, User *user);
void buyer_menu(Catalog *catalog, InquiryQueue *q, ShoppingCart *cart, TxStack *stack, HashTable *ht, UserList *users, UserHashTable *user_ht, User *user);
void browse_menu(Catalog *catalog);
int display_inquiries_for_seller(InquiryQueue *q, const char *seller_name);
InquiryNode *get_seller_inquiry_by_index(InquiryQueue *q, const char *seller_name, int index);

    // --- Forward declarations for seller features ---
void seller_add_item_step_by_step(Catalog *catalog, InquiryQueue *q, HashTable *ht, User *user);
void seller_view_listings(Catalog *catalog, InquiryQueue *q, HashTable *ht, User *user);
void seller_remove_item(Catalog *catalog, InquiryQueue *q, User *user);
void seller_update_item(Catalog *catalog, InquiryQueue *q, User *user);
static int validate_contact(const char *contact); /* (+63) or +63 then 10 digits */

    // --- Forward declarations for user auth ---
static int validate_password(const char *password);
static int validate_username(const char *username);
int startup_menu(UserList *users, UserHashTable *user_ht, User **current_user);
void init_user_list(UserList *list);
void init_user_hash(UserHashTable *ht);
int login_user(UserHashTable *ht, User **user);
int register_user(UserList *list, UserHashTable *ht, User **user);
User *user_ht_lookup(UserHashTable *ht, const char *username);
User *add_user(UserList *list, UserHashTable *ht,
               const char *username, const char *password, const char *display_name, char role);
void free_user_data(UserList *list, UserHashTable *ht);
void seed_demo_users(UserList *list, UserHashTable *ht);
void seed_demo_catalog(Catalog *catalog, HashTable *ht);

static int prompt_quantity(void)
{
    int quantity = 0;
    while (1)
    {
        printf("  Enter quantity: ");
        if (scanf("%d", &quantity) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Quantity must be a number.\n");
            continue;
        }
        clear_input_buffer();
        if (quantity <= 0)
        {
            printf("  [ERROR] Quantity must be at least 1.\n");
            continue;
        }
        return quantity;
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(void)
{
    // Initialize all data structures
    Catalog catalog;
    InquiryQueue queue;
    TxStack history;
    HashTable ht;
    ShoppingCart cart;
    UserList users;
    UserHashTable user_ht;
    User *current_user = NULL;

    init_catalog(&catalog);
    init_queue(&queue);
    init_stack(&history);
    init_hash_table(&ht);
    init_cart(&cart);
    init_user_list(&users);
    init_user_hash(&user_ht);

    seed_demo_users(&users, &user_ht);
    seed_demo_catalog(&catalog, &ht);

    // Startup menu loop
    while (1)
    {
        int choice = startup_menu(&users, &user_ht, &current_user);
        if (choice == 0)
            break; /* exit application */

        if (current_user != NULL)
        {
            if (current_user->role == 'B')
            {
                buyer_menu(&catalog, &queue, &cart, &history, &ht, &users, &user_ht, current_user);
            }
            else if (current_user->role == 'S')
            {
                seller_menu(&catalog, &queue, &history, &ht, &users, &user_ht, current_user);
            }
            current_user = NULL; /* on logout, return to startup */
        }
    }

    // Free all allocated memory before exit
    free_catalog(&catalog);
    free_queue(&queue);
    free_stack(&history);
    free_hash_table(&ht);
    free_cart(&cart);
    free_user_data(&users, &user_ht);

    printf("\n  Thank you for using Trade Hub!\n\n");
    return 0;
}

void seed_demo_users(UserList *list, UserHashTable *ht)
{
    add_user(list, ht, "maria101", "6543DF", "Maria Santos", 'S');
    add_user(list, ht, "gelo321", "9090LA", "Angelo Lorio", 'B');
}

void seed_demo_catalog(Catalog *catalog, HashTable *ht)
{
    int id;

    id = post_item_silent(catalog, "Blouse", "uniform", "Brand New", 350.00f,
                          "Maria Santos", "09151234001", 5);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Blouse", "uniform", "Good Condition", 250.00f,
                          "Rosa Garcia", "09151234005", 3);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Blouse", "uniform", "Brand New", 380.00f,
                          "Elena Santos", "09151234009", 2);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Pants", "uniform", "Like New", 280.00f,
                          "Juan Reyes", "09151234002", 3);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Pants", "uniform", "Brand New", 320.00f,
                          "Pedro Martinez", "09151234005", 2);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Pants", "uniform", "Good Condition", 220.00f,
                          "Diego Cruz", "09151234009", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Black Shoes", "uniform", "Good Condition", 200.00f,
                          "Ana Cruz", "09151234010", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Black Shoes", "uniform", "Like New", 180.00f,
                          "Sofia Reyes", "09151234011", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Necktie", "uniform", "Brand New", 120.00f,
                          "Carlos Lopez", "09151234012", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Necktie", "uniform", "Good Condition", 90.00f,
                          "Miguel Torres", "09151234013", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Type A Jacket", "uniform", "Brand New", 450.00f,
                          "Lt. Garcia", "09151234014", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Type A Jacket", "uniform", "Like New", 420.00f,
                          "Cpt. Morales", "09151234015", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Combat Shoes", "uniform", "Like New", 380.00f,
                          "Sgt. Ramos", "09151234016", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Combat Shoes", "uniform", "Good Condition", 350.00f,
                          "Lt. Cruz", "09151234017", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Type C T-Shirt", "uniform", "Good Condition", 180.00f,
                          "Cadet Lopez", "09151234018", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Type C T-Shirt", "uniform", "Brand New", 200.00f,
                          "Cadet Reyes", "09151234019", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Bullcap", "uniform", "Brand New", 150.00f,
                          "Maj. Santos", "09151234020", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Bullcap", "uniform", "Like New", 130.00f,
                          "Maj. Garcia", "09151234021", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Garrison Belt", "uniform", "Good Condition", 120.00f,
                          "Sgt. Dela Cruz", "09151234022", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "Garrison Belt", "uniform", "Brand New", 140.00f,
                          "Sgt. Torres", "09151234023", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE T-Shirt", "uniform", "Brand New", 220.00f,
                          "Coach Rivera", "09151234024", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE T-Shirt", "uniform", "Good Condition", 180.00f,
                          "Maria Sports", "09151234025", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE T-Shirt", "uniform", "Like New", 240.00f,
                          "Coach Santos", "09151234026", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE T-Shirt", "uniform", "Brand New", 250.00f,
                          "Carlos Gym", "09151234027", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE T-Shirt", "uniform", "Good Condition", 190.00f,
                          "Miguel Coach", "09151234028", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE Pants", "uniform", "Like New", 260.00f,
                          "Juan PE Teacher", "09151234029", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE Pants", "uniform", "Brand New", 280.00f,
                          "Luis Fitness", "09151234030", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE Pants", "uniform", "Good Condition", 230.00f,
                          "Ana Fitness", "09151234031", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "PE Pants", "uniform", "Like New", 270.00f,
                          "Rosa Sports", "09151234032", 1);
    if (id != -1)
        ht_insert(ht, "uniform", id);

    id = post_item_silent(catalog, "School Bag", "supply", "Brand New", 450.00f,
                          "Maria Gonzalez", "09151234033", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "School Bag", "supply", "Like New", 400.00f,
                          "Carlos Supplies", "09151234034", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Calculator", "supply", "Like New", 380.00f,
                          "Prof. Alvarez", "09151234035", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Calculator", "supply", "Good Condition", 320.00f,
                          "Science Dept", "09151234036", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Bond Paper", "supply", "Brand New", 80.00f,
                          "Student Council", "09151234037", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Bond Paper", "supply", "Brand New", 90.00f,
                          "Library", "09151234038", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Ruler (30cm)", "supply", "Brand New", 30.00f,
                          "Art Teacher", "09151234039", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Ruler (30cm)", "supply", "Like New", 25.00f,
                          "Math Teacher", "09161234040", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Notebook", "supply", "Good Condition", 50.00f,
                          "Anna Store", "09161234041", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Notebook", "supply", "Brand New", 60.00f,
                          "Book Store", "09161234042", 1);
    if (id != -1)
        ht_insert(ht, "supply", id);

    id = post_item_silent(catalog, "Algebra Textbook", "book", "Like New", 300.00f,
                          "Senior Student", "09161234043", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Chemistry Advanced", "book", "Good Condition", 350.00f,
                          "Faculty Member", "09911234567", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "English Literature", "book", "Like New", 280.00f,
                          "Book Collector", "09161234045", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Biology Textbook", "book", "Brand New", 400.00f,
                          "Paulo Books", "09161234046", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Geometry Textbook", "book", "Good Condition", 320.00f,
                          "Math Student", "09161234047", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Physics Textbook", "book", "Like New", 380.00f,
                          "Science Teacher", "09161234048", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Grammar Book", "book", "Brand New", 250.00f,
                          "English Dept", "09161234049", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "World History", "book", "Good Condition", 290.00f,
                          "History Teacher", "09161234050", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "Programming Book", "book", "Like New", 350.00f,
                          "Computer Lab", "09161234051", 1);
    if (id != -1)
        ht_insert(ht, "book", id);

    id = post_item_silent(catalog, "USB Flash Drive", "other", "Like New", 250.00f,
                          "IT Office", "09161234052", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "USB Flash Drive", "other", "Brand New", 320.00f,
                          "Computer Lab", "09161234053", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Printer Ink", "other", "Brand New", 450.00f,
                          "Admin Office", "09161234054", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Printer Ink", "other", "Brand New", 500.00f,
                          "Registrar Staff", "09161234055", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Extension Cord", "other", "Good Condition", 220.00f,
                          "Facilities Staff", "09161234056", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Extension Cord", "other", "Brand New", 280.00f,
                          "Electrical Club", "09161234057", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Clipboard", "other", "Brand New", 75.00f,
                          "Guidance Office", "09161234058", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Clipboard", "other", "Like New", 55.00f,
                          "Class Adviser", "09161234059", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "USB Flash Drive", "other", "Good Condition", 200.00f,
                          "Security Office", "09161234060", 1);
    if (id != -1)
        ht_insert(ht, "other", id);

    id = post_item_silent(catalog, "Printer Ink", "other", "Like New", 380.00f,
                          "Admin Office", "09161234061", 1);
    if (id != -1)
        ht_insert(ht, "other", id);
}

/* ============================================================
   USER AUTHENTICATION
   ============================================================ */

void init_user_list(UserList *list)
{
    list->head = NULL;
}

void init_user_hash(UserHashTable *ht)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        ht->buckets[i] = NULL;
    }
}

User *user_ht_lookup(UserHashTable *ht, const char *username)
{
    int index = hash_category(username);
    UserHashNode *current = ht->buckets[index];

    while (current != NULL)
    {
        if (strcasecmp(current->username, username) == 0)
        {
            return current->user;
        }
        current = current->next;
    }

    return NULL;
}

User *add_user(UserList *list, UserHashTable *ht,
               const char *username, const char *password, const char *display_name, char role)
{
    if (user_ht_lookup(ht, username) != NULL)
    {
        return NULL; /* Username already exists */
    }

    User *new_user = (User *)malloc(sizeof(User));
    if (!new_user)
    {
        return NULL;
    }

    strncpy(new_user->username, username, sizeof(new_user->username) - 1);
    new_user->username[sizeof(new_user->username) - 1] = '\0';
    strncpy(new_user->password, password, sizeof(new_user->password) - 1);
    new_user->password[sizeof(new_user->password) - 1] = '\0';
    strncpy(new_user->display_name, display_name, sizeof(new_user->display_name) - 1);
    new_user->display_name[sizeof(new_user->display_name) - 1] = '\0';
    new_user->role = role;
    new_user->next = list->head;
    list->head = new_user;

    UserHashNode *new_node = (UserHashNode *)malloc(sizeof(UserHashNode));
    if (!new_node)
    {
        free(new_user);
        return NULL;
    }

    strncpy(new_node->username, username, sizeof(new_node->username) - 1);
    new_node->username[sizeof(new_node->username) - 1] = '\0';
    new_node->user = new_user;
    int index = hash_category(username);
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;

    return new_user;
}

static void print_welcome_header(void)
{
    printf("\n");
    printf("================================================================================\n");
    printf("                           T R A D E   H U B\n");
    printf("                  Buy and Sell School Uniforms and Supplies\n");
    printf("                         CBSUA Sipocot Campus\n");
    printf("================================================================================\n");
    printf("\n");
}

static void print_login_header(void)
{
    printf("\n");
    printf("================================================================================\n");
    printf("%*sLOGIN\n", 39, "");
    printf("================================================================================\n");
    printf("\n");
}

static void print_register_header(void)
{
    printf("\n");
    printf("================================================================================\n");
    printf("%*sREGISTER\n", 37, "");
    printf("================================================================================\n");
    printf("\n");
}

int startup_menu(UserList *users, UserHashTable *user_ht, User **current_user)
{
    int choice;

    while (1)
    {
        print_welcome_header();
        printf("  [1]  Login\n");
        printf("  [2]  Register\n");
        printf("  [0]  Exit\n");
        printf("\n");
        printf("================================================================================\n");
        printf("  Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            continue;
        }
        clear_input_buffer();

        if (choice == 1)
        {
            int result = login_user(user_ht, current_user);
            if (result == 1)
            {
                return 1; /* user logged in successfully */
            }
    // otherwise return to startup menu
        }
        else if (choice == 2)
        {
            int result = register_user(users, user_ht, current_user);
            if (result == 1)
            {
                return 1; /* user registered and logged in successfully */
            }
    // otherwise return to startup menu
        }
        else if (choice == 0)
        {
            return 0; /* exit application */
        }
        else
        {
            printf("  [ERROR] Invalid choice.\n");
        }
    }
}

int login_user(UserHashTable *ht, User **user)
{
    char username[50];
    char password[50];
    int max_attempts = 3;
    int attempt = 0;

    while (attempt < max_attempts)
    {
        print_login_header();

        printf("  Username [0-Back]: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';

        if (strcmp(username, "0") == 0)
        {
            return 0; /* Back to startup menu */
        }

        printf("  Password [0-Back]: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';

        if (strcmp(password, "0") == 0)
        {
            return 0; /* Back to startup menu */
        }

        printf("\n");
        printf("================================================================================\n");

        *user = user_ht_lookup(ht, username);
        if (*user != NULL && strcmp((*user)->password, password) == 0)
        {
    // Login successful
            print_login_header();
            printf("  Welcome back, %s!\n", (*user)->display_name);
            char role_str[20] = "";
            if ((*user)->role == 'B')
                strcpy(role_str, "Buyer");
            else if ((*user)->role == 'S')
                strcpy(role_str, "Seller");
            printf("  Logged in as: %s (%s)\n", (*user)->display_name, role_str);
            printf("\n");
            printf("================================================================================\n");
            printf("  Press any key to continue...\n");
            getchar();
            return 1; /* Success */
        }
        else
        {
            attempt++;
            if (attempt >= max_attempts)
            {
    // Max attempts reached - return to startup
                return 0;
            }
            
            print_login_header();
            printf("  Incorrect username or password.\n");
            printf("  Attempts : %d of %d\n", attempt, max_attempts);
            printf("\n");
        }
    }

    return 0; /* Return to startup menu */
}

int register_user(UserList *users, UserHashTable *ht, User **user)
{
    char username[50];
    char password[50];
    char display_name[50];
    char role_input[10];

    print_register_header();

    printf("  Username [0-Back]: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    if (strcmp(username, "0") == 0)
    {
        return 0; /* Back to startup menu */
    }

    if (strlen(username) == 0)
    {
        printf("  [ERROR] Username cannot be empty.\n");
        return 0;
    }

    if (!validate_username(username))
    {
        printf("  [ERROR] Username must be at least 6 characters and contain both letters and numbers.\n");
        printf("         Example: user123\n");
        return 0;
    }

    if (user_ht_lookup(ht, username) != NULL)
    {
        printf("  [ERROR] Username '%s' already exists.\n", username);
        return 0;
    }

    printf("  Password [0-Back]: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    if (strcmp(password, "0") == 0)
    {
        return 0; /* Back to startup menu */
    }

    if (strlen(password) == 0)
    {
        printf("  [ERROR] Password cannot be empty.\n");
        return 0;
    }

    if (!validate_password(password))
    {
        printf("  [ERROR] Password must be at least 6 characters and contain both letters and numbers.\n");
        printf("         Example: Password123\n");
        return 0;
    }

    printf("  Full Name[0-Back]: ");
    fgets(display_name, sizeof(display_name), stdin);
    display_name[strcspn(display_name, "\n")] = '\0';

    if (strcmp(display_name, "0") == 0)
    {
        return 0; /* Back to startup menu */
    }

    if (strlen(display_name) == 0)
    {
        printf("  [ERROR] Full name cannot be empty.\n");
        return 0;
    }

    printf("  Role     [S/B]  : ");
    fgets(role_input, sizeof(role_input), stdin);
    role_input[strcspn(role_input, "\n")] = '\0';

    if (strcmp(role_input, "0") == 0)
    {
        return 0; /* Back to startup menu */
    }

    if (strlen(role_input) != 1)
    {
        printf("  [ERROR] Enter S or B.\n");
        return 0;
    }

    char role = toupper((unsigned char)role_input[0]);
    if (role != 'S' && role != 'B')
    {
        printf("  [ERROR] Enter S or B.\n");
        return 0;
    }

    printf("\n");
    printf("================================================================================\n");

    // Create the new user
    *user = add_user(users, ht, username, password, display_name, role);
    if (!*user)
    {
        printf("  [ERROR] Registration failed. Try again.\n");
        return 0;
    }

    // Registration successful
    print_register_header();
    printf("  Account created successfully!\n");
    printf("  Welcome to Trade Hub, %s!\n", display_name);
    char role_str[20] = "";
    if (role == 'B')
        strcpy(role_str, "Buyer");
    else if (role == 'S')
        strcpy(role_str, "Seller");
    printf("  You are now logged in as: %s (%s)\n", display_name, role_str);
    printf("\n");
    printf("================================================================================\n");
    printf("  Press any key to continue...\n");
    getchar();
    return 1; /* Success - user is now logged in */
}


void free_user_data(UserList *list, UserHashTable *ht)
{
    UserHashNode *current_node;
    UserHashNode *next_node;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        current_node = ht->buckets[i];
        while (current_node != NULL)
        {
            next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
        ht->buckets[i] = NULL;
    }

    User *current_user = list->head;
    while (current_user != NULL)
    {
        User *next_user = current_user->next;
        free(current_user);
        current_user = next_user;
    }
    list->head = NULL;
}

/* ============================================================
   DELETE ACCOUNT FUNCTION
   ============================================================ */
int delete_account(UserList *users, UserHashTable *user_ht, User *user, Catalog *catalog)
{
    if (user == NULL)
        return 0;

    printf("\n");
    printf("================================================================================\n");
    printf("  WARNING: This action will permanently delete your account!\n");
    printf("  - All your information will be removed\n");
    if (user->role == 'S')
        printf("  - All your product listings will be removed\n");
    printf("  This action cannot be undone.\n");
    printf("================================================================================\n");
    printf("\n");
    printf("  Enter your password to confirm account deletion: ");
    
    char password[50];
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';
    
    if (strcmp(user->password, password) != 0)
    {
        printf("  [ERROR] Incorrect password. Account deletion cancelled.\n");
        printf("  Press any key to continue...\n");
        getchar();
        return 0;
    }

    printf("  Type 'DELETE' to confirm: ");
    char confirm[50];
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';
    trim_input(confirm);

    if (strcasecmp(confirm, "DELETE") != 0)
    {
        printf("  Account deletion cancelled.\n");
        printf("  Press any key to continue...\n");
        getchar();
        return 0;
    }

    /* Remove user's items from catalog if seller */
    if (user->role == 'S')
    {
        Item *current = catalog->head;
        while (current != NULL)
        {
            Item *next = current->next;
            if ((strcasecmp(current->seller, user->username) == 0 ||
                 strcasecmp(current->seller, user->display_name) == 0) &&
                !current->is_sold)
            {
                current->is_sold = 1; /* Mark as sold/removed */
            }
            current = next;
        }
    }

    /* Remove from UserHashTable */
    int index = hash_category(user->username);
    UserHashNode *current_node = user_ht->buckets[index];
    UserHashNode *prev_node = NULL;

    while (current_node != NULL)
    {
        if (strcasecmp(current_node->username, user->username) == 0)
        {
            if (prev_node != NULL)
            {
                prev_node->next = current_node->next;
            }
            else
            {
                user_ht->buckets[index] = current_node->next;
            }
            free(current_node);
            break;
        }
        prev_node = current_node;
        current_node = current_node->next;
    }

    /* Remove from UserList */
    User *current_user = users->head;
    User *prev_user = NULL;

    while (current_user != NULL)
    {
        if (strcmp(current_user->username, user->username) == 0)
        {
            if (prev_user != NULL)
            {
                prev_user->next = current_user->next;
            }
            else
            {
                users->head = current_user->next;
            }
            free(current_user);
            break;
        }
        prev_user = current_user;
        current_user = current_user->next;
    }

    printf("\n");
    printf("  [SUCCESS] Your account has been deleted successfully.\n");
    printf("  You will be returned to the login screen.\n");
    printf("  Press any key to continue...\n");
    getchar();
    
    return 1; /* Signal to return to startup menu */
}

/* ============================================================
   MAIN MENU
   ============================================================ */
void main_menu(Catalog *catalog, InquiryQueue *q,
               TxStack *stack, HashTable *ht, ShoppingCart *cart, UserList *users, UserHashTable *user_ht, User *user)
{
    int choice;
    const char *role_name = (user && user->role == 'S') ? "Seller" : "Buyer";

    do
    {
        print_header("MAIN MENU");
        if (user != NULL)
        {
            printf("  Logged in as: %s (%s)\n", user->display_name, role_name);
            printf("  ----------------------------------------\n");
        }
        if (user->role == 'S')
        {
            printf("  [1] I am a SELLER\n");
        }
        else
        {
            printf("  [2] I am a BUYER\n");
        }
        printf("  [3] View Transaction History\n");
        printf("  [4] View Category Index\n");
        printf("\n  Enter choice [0-Back]: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            continue;
        }
        clear_input_buffer();

        switch (choice)
        {
        case 1:
            if (user->role == 'S')
            {
                seller_menu(catalog, q, stack, ht, users, user_ht, user);
            }
            else
            {
                printf("  [ERROR] Access denied. You are not a seller.\n");
            }
            break;
        case 2:
            if (user->role == 'B')
            {
                buyer_menu(catalog, q, cart, stack, ht, users, user_ht, user);
            }
            else
            {
                printf("  [ERROR] Access denied. You are not a buyer.\n");
            }
            break;
        case 3:
            print_header("TRANSACTION HISTORY");
            display_history(stack);
            break;
        case 4:
            while (1)
            {
                display_hash_table(ht);
                printf("                           [V] View Category                                  \n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char cat_choice[32];
                if (!fgets(cat_choice, sizeof(cat_choice), stdin))
                {
                    break;
                }
                trim_input(cat_choice);

                if (strcasecmp(cat_choice, "B") == 0 || strcmp(cat_choice, "0") == 0)
                {
                    break;
                }
                else if (strcasecmp(cat_choice, "V") == 0)
                {
                    printf("  Enter No. to Browse Category: ");
                    char cat_input[32];
                    if (!fgets(cat_input, sizeof(cat_input), stdin))
                    {
                        continue;
                    }
    // TODO: Add browse category functionality
                    printf("  [INFO] Browse Category feature coming soon.\n");
                    printf("  Press Enter to continue...");
                    char dummy[32];
                    fgets(dummy, sizeof(dummy), stdin);
                    continue;
                }
                else
                {
                    printf("  [ERROR] Invalid choice. Enter V or 0.\n");
                }
            }
            break;
        case 0:
            break;
        default:
            printf("  [ERROR] Invalid choice. Try again.\n");
        }

    } while (choice != 0);
}

/* ============================================================
   SELLER MENU
   ============================================================ */

    // --- Forward declarations for seller functions ---
void seller_view_listings(Catalog *catalog, InquiryQueue *q, HashTable *ht, User *user);
void seller_remove_item(Catalog *catalog, InquiryQueue *q, User *user);
int count_pending_orders_for_item(InquiryQueue *q, int item_id);
Item *get_item_by_id_for_seller(Catalog *catalog, int item_id, const char *seller);
void seller_view_incoming_orders(Catalog *catalog, TxStack *stack, User *user);
void seller_sales_history(TxStack *stack, User *user);
int delete_account(UserList *users, UserHashTable *user_ht, User *user, Catalog *catalog);

void seller_menu(Catalog *catalog, InquiryQueue *q,
                 TxStack *stack, HashTable *ht, UserList *users, UserHashTable *user_ht, User *user)
{
    int choice;

    do
    {
        print_header("SELLER MENU");
        printf("  Logged in as: %s (Seller)\n", user->display_name);
        printf("  [1] Add Product\n");
        printf("  [2] View My Listings\n");
        printf("  [3] View Incoming Orders\n");
        printf("  [4] Sales History\n");
        printf("  [5] Delete Account\n");
        printf("  [6] Logout\n");
        printf("\n  Enter choice [0-Back]: ");

        if (scanf("%d", &choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            continue;
        }
        clear_input_buffer();

        if (choice == 1)
        {
    // --- Add Product ---
            seller_add_item_step_by_step(catalog, q, ht, user);
        }
        else if (choice == 2)
        {
    // --- View My Listings ---
            seller_view_listings(catalog, q, ht, user);
        }
        else if (choice == 3)
        {
    // --- View Incoming Orders (new flow) ---
            seller_view_incoming_orders(catalog, stack, user);
        }
        else if (choice == 4)
        {
    // --- Sales History ---
            seller_sales_history(stack, user);
        }
        else if (choice == 5)
        {
    // --- Delete Account ---
            if (delete_account(users, user_ht, user, catalog) == 1)
            {
                break; /* Account deleted, return to startup */
            }
        }
        else if (choice == 6)
        {
    // --- Logout ---
            printf("Are you sure you want to logout? (y/n): ");
            char confirm[10];
            if (fgets(confirm, sizeof(confirm), stdin))
            {
                trim_input(confirm);
                if (confirm[0] == 'y' || confirm[0] == 'Y')
                {
                    break;
                }
            }
        }
        else if (choice == 0)
        {
    // --- Back to Main Menu ---
            break;
        }
        else
        {
            printf("  [ERROR] Invalid choice. Please select 1, 2, 3, 4, 5, 6, or 0.\n");
            printf("  Press any key to continue...");
            getchar();
        }
    } while (choice != 0);
}

    // --- Seller View Listings with Options ---
void seller_view_listings(Catalog *catalog, InquiryQueue *q, HashTable *ht, User *user)
{
    while (1)
    {
        print_centered_header("MY LISTINGS");
        printf("  Logged in as: %s (Seller)\n", user->display_name);
        printf("--------------------------------------------------------------------------------\n");
        printf("  No.  ID      Product Name     Condition        Price        Stock    Status\n");
        printf("  ------------------------------------------------------------------------------\n");

        Item *current = catalog->head;
        int count = 0;
        int has_listings = 0;

        while (current != NULL)
        {
            if (!current->is_sold &&
                (strcasecmp(current->seller, user->username) == 0 ||
                 strcasecmp(current->seller, user->display_name) == 0))
            {
                has_listings = 1;
                count++;
                char stock_str[20];
                sprintf(stock_str, "%d pcs", current->quantity);
                printf("  [%2d]  %-6d  %-15s %-15s PHP %-8.2f %-8s Active\n",
                       count, current->id, current->title, current->condition,
                       current->price, stock_str);
            }
            current = current->next;
        }

        if (!has_listings)
        {
            printf("\n                     You have no active listings.\n");
            printf("                   Add a product to start selling!\n\n");
            printf("================================================================================\n");
            printf("  [A] Add Product\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char choice;
            if (scanf(" %c", &choice) != 1)
            {
                clear_input_buffer();
                continue;
            }
            clear_input_buffer();
            choice = toupper(choice);

            if (choice == 'A')
            {
                seller_add_item_step_by_step(catalog, q, ht, user);
                continue;
            }
            else if (choice == 'B' || choice == '0')
            {
                return;
            }
            continue;
        }

        printf("  ------------------------------------------------------------------------------\n");
        printf("  Total Listings: %d item(s)\n", count);
        printf("================================================================================\n");
        printf("  [A] Add Product   [R] Remove Item   [U] Update Item\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

      char choice;
        if (scanf(" %c", &choice) != 1)
        {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        choice = toupper(choice);

        if (choice == 'A')
        {
            seller_add_item_step_by_step(catalog, q, ht, user);
            continue;
        }
        else if (choice == 'R')
        {
            seller_remove_item(catalog, q, user);
        }
        else if (choice == 'U')
        {
            seller_update_item(catalog, q, user);
        }
        else if (choice == 'B' || choice == '0')
        {
            return;
        }
        else
        {
            printf("  [ERROR] Invalid choice. Please select A, R, U, or 0.\n");
            printf("  Press any key to continue...");
            getchar();
        }
    }
}

    // --- Seller Remove Item Flow ---
void seller_remove_item(Catalog *catalog, InquiryQueue *q, User *user)
{
    print_centered_header("REMOVE ITEM");
    printf("  Enter No. to Remove [0-Back]: ");

    int selection;
    if (scanf("%d", &selection) != 1)
    {
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (selection == 0)
        return;

    // Find the item by selection number for this seller
    Item *current = catalog->head;
    int count = 0;
    Item *selected_item = NULL;

    while (current != NULL)
    {
        if (!current->is_sold &&
            (strcasecmp(current->seller, user->username) == 0 ||
             strcasecmp(current->seller, user->display_name) == 0))
        {
            count++;
            if (count == selection)
            {
                selected_item = current;
                break;
            }
        }
        current = current->next;
    }

    if (!selected_item)
    {
        printf("  [ERROR] Invalid selection.\n");
        printf("  Press any key to continue...");
        getchar();
        return;
    }

    // Check for pending orders
    int pending_orders = count_pending_orders_for_item(q, selected_item->id);
    if (pending_orders > 0)
    {
        print_centered_header("!! CANNOT REMOVE ITEM");
        printf("  Item     : %s (ID: %d)\n", selected_item->title, selected_item->id);
        printf("  Reason   : This item has a pending order.\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  Pending Orders : %d\n", pending_orders);
        printf("  Please wait for all orders to be\n");
        printf("  completed or cancelled before removing.\n");
        printf("================================================================================\n");
        printf("  [V] View Pending Orders\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        char choice;
        if (scanf(" %c", &choice) != 1)
        {
            clear_input_buffer();
            return;
        }
        clear_input_buffer();
        choice = toupper(choice);

        if (choice == 'V')
        {
            // Show pending orders for this item
            printf("\n  Pending orders for %s:\n", selected_item->title);
            InquiryNode *inq_current = q->front;
            int inq_count = 0;
            while (inq_current != NULL)
            {
                if (inq_current->item_id == selected_item->id)
                {
                    inq_count++;
                    printf("  %d. Buyer: %s, Message: %s\n", inq_count,
                           inq_current->buyer_name, inq_current->message);
                }
                inq_current = inq_current->next;
            }
            printf("  Press any key to continue...");
            getchar();
        }
        return;
    }

    // Safe to remove - show confirmation
    print_centered_header("REMOVE ITEM");
    printf("  Are you sure you want to remove this item?\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("  ID           : %d\n", selected_item->id);
    printf("  Product Name : %s\n", selected_item->title);
    printf("  Condition    : %s\n", selected_item->condition);
    printf("  Price        : PHP %.2f\n", selected_item->price);
    printf("  Stock        : %d pcs\n", selected_item->quantity);
    printf("  Status       : Active\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("  !! This action cannot be undone.\n");
    printf("================================================================================\n");
    printf("  [Y] Yes, Remove   [N] No, Keep It\n");
    printf("================================================================================\n");
    printf("  Enter choice [0-Back]: ");

    char confirm;
    if (scanf(" %c", &confirm) != 1)
    {
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    confirm = toupper(confirm);

    if (confirm == 'Y')
    {
        // Remove the item
        if (remove_item(catalog, selected_item->id))
        {
            char timestamp[64];
            get_timestamp(timestamp, sizeof(timestamp));

            print_centered_header("REMOVE ITEM");
            printf("  Item successfully removed from your listings!\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  ID           : %d\n", selected_item->id);
            printf("  Product Name : %s\n", selected_item->title);
            printf("  Condition    : %s\n", selected_item->condition);
            printf("  Price        : PHP %.2f\n", selected_item->price);
            printf("  Date Removed : %s\n", timestamp);
            printf("================================================================================\n");
            printf("  [A] Add New Product   [V] View My Listings\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char next_choice;
            if (scanf(" %c", &next_choice) != 1)
            {
                clear_input_buffer();
                return;
            }
            clear_input_buffer();
            next_choice = toupper(next_choice);

            if (next_choice == 'A')
            {
                return; /* seller_menu */
            }
            else if (next_choice == 'V')
            {
                return;
            }
            else if (next_choice == '0')
            {
                return;
            }
        }
        else
        {
            printf("  [ERROR] Failed to remove item.\n");
        }
    }
    else if (confirm == 'N')
    {
        printf("================================================================================\n");
        printf("  Item #%d %s was NOT removed.\n", selected_item->id, selected_item->title);
        printf("  Your listing remains Active.\n");
        printf("================================================================================\n");
        printf("  Press any key to go back...");
        getchar();
    }
}

    // --- Seller Update Item Flow ---
void seller_update_item(Catalog *catalog, InquiryQueue *q, User *user)
{
    char input[128];
    Item *current = catalog->head;
    Item *selected_item = NULL;
    int count = 0;
    int selection = 0;

    print_centered_header("UPDATE ITEM");
    printf("  Logged in as: %s (Seller)\n", user->display_name);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Enter No. to Update [0-Back]: ");
    if (!fgets(input, sizeof(input), stdin))
        return;
    trim_input(input);
    if (strcmp(input, "0") == 0)
        return;

    selection = atoi(input);
    if (selection <= 0)
        return;

    while (current != NULL)
    {
        if (!current->is_sold &&
            (strcasecmp(current->seller, user->username) == 0 ||
             strcasecmp(current->seller, user->display_name) == 0))
        {
            count++;
            if (count == selection)
            {
                selected_item = current;
                break;
            }
        }
        current = current->next;
    }

    if (!selected_item)
        return;

    // Working copy until user confirms save.
    char new_title[100];
    char new_category[50];
    char new_condition[20];
    char new_contact[50];
    float new_price;
    int new_stock;

    strncpy(new_title, selected_item->title, sizeof(new_title) - 1);
    new_title[sizeof(new_title) - 1] = '\0';
    strncpy(new_category, selected_item->category, sizeof(new_category) - 1);
    new_category[sizeof(new_category) - 1] = '\0';
    strncpy(new_condition, selected_item->condition, sizeof(new_condition) - 1);
    new_condition[sizeof(new_condition) - 1] = '\0';
    strncpy(new_contact, selected_item->contact, sizeof(new_contact) - 1);
    new_contact[sizeof(new_contact) - 1] = '\0';
    new_price = selected_item->price;
    new_stock = selected_item->quantity;

    while (1)
    {
        print_centered_header("UPDATE ITEM");
        printf("  Item ID  : %d\n", selected_item->id);
        printf("--------------------------------------------------------------------------------\n");
        printf("  Current Details:\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  [1] Product Name : %s\n", new_title);
        printf("  [2] Category     : %s > %s\n", new_category, new_title);
        printf("  [3] Condition    : %s\n", new_condition);
        printf("  [4] Price        : PHP %.2f\n", new_price);
        printf("  [5] Stock        : %d pcs\n", new_stock);
        printf("  [6] Contact      : %s\n", new_contact);
        printf("--------------------------------------------------------------------------------\n");
        printf("  Select field to update [0-Back]: ");
        if (!fgets(input, sizeof(input), stdin))
            return;
        trim_input(input);
        if (strcmp(input, "0") == 0)
            return;

        if (strcmp(input, "1") == 0)
        {
            print_centered_header("UPDATE PRODUCT NAME");
            printf("  Current : %s\n", new_title);
            printf("  New     [0-Back]: ");
            if (!fgets(input, sizeof(input), stdin))
                return;
            trim_input(input);
            if (strcmp(input, "0") == 0)
                continue;
            if (strlen(input) == 0)
                continue;
            strncpy(new_title, input, sizeof(new_title) - 1);
            new_title[sizeof(new_title) - 1] = '\0';
        }
        else if (strcmp(input, "2") == 0)
        {
            print_centered_header("UPDATE CATEGORY");
            printf("  Current : %s > %s\n", new_category, new_title);
            printf("--------------------------------------------------------------------------------\n");
            printf("  [1] Uniform\n");
            printf("  [2] Supply\n");
            printf("  [3] Book\n");
            printf("  [4] Other\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");
            if (!fgets(input, sizeof(input), stdin))
                return;
            trim_input(input);
            if (strcmp(input, "0") == 0)
                continue;
            if (strcmp(input, "1") == 0)
                strcpy(new_category, "Uniform");
            else if (strcmp(input, "2") == 0)
                strcpy(new_category, "Supply");
            else if (strcmp(input, "3") == 0)
                strcpy(new_category, "Book");
            else if (strcmp(input, "4") == 0)
                strcpy(new_category, "Other");
            else
                continue;
        }
        else if (strcmp(input, "3") == 0)
        {
            print_centered_header("UPDATE CONDITION");
            printf("  Current : %s\n", new_condition);
            printf("--------------------------------------------------------------------------------\n");
            printf("  [1] Brand New\n");
            printf("  [2] Good Condition\n");
            printf("  [3] Worn\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");
            if (!fgets(input, sizeof(input), stdin))
                return;
            trim_input(input);
            if (strcmp(input, "0") == 0)
                continue;
            if (strcmp(input, "1") == 0)
                strcpy(new_condition, "Brand New");
            else if (strcmp(input, "2") == 0)
                strcpy(new_condition, "Good Condition");
            else if (strcmp(input, "3") == 0)
                strcpy(new_condition, "Worn");
            else
                continue;
        }
        else if (strcmp(input, "4") == 0)
        {
            int pending_orders = count_pending_orders_for_item(q, selected_item->id);
            if (pending_orders > 0)
            {
                print_centered_header("!! WARNING");
                printf("  Item #%d has %d pending order(s).\n", selected_item->id, pending_orders);
                printf("  Updating price may affect ongoing transactions.\n");
                printf("--------------------------------------------------------------------------------\n");
                printf("  Do you still want to update the price?\n");
                printf("  [Y] Yes, Continue   [N] No, Go Back\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");
                if (!fgets(input, sizeof(input), stdin))
                    return;
                trim_input(input);
                if (!(strlen(input) == 1 && toupper((unsigned char)input[0]) == 'Y'))
                    continue;
            }

            while (1)
            {
                float parsed = 0.0f;
                print_centered_header("UPDATE PRICE");
                printf("  Current : PHP %.2f\n", new_price);
                printf("  New     [0-Back]: PHP ");
                if (!fgets(input, sizeof(input), stdin))
                    return;
                trim_input(input);
                if (strcmp(input, "0") == 0)
                    break;
                if (sscanf(input, "%f", &parsed) != 1 || parsed <= 0.0f)
                {
                    print_centered_header("!! INPUT ERROR");
                    printf("  Price cannot be 0 or negative.\n");
                    printf("  Please enter a valid price.\n");
                    printf("--------------------------------------------------------------------------------\n");
                    printf("  Enter Price (PHP) [0-Back]: ");
                    if (!fgets(input, sizeof(input), stdin))
                        return;
                    trim_input(input);
                    if (strcmp(input, "0") == 0)
                        break;
                    if (sscanf(input, "%f", &parsed) != 1 || parsed <= 0.0f)
                        continue;
                }
                new_price = parsed;
                break;
            }
        }
        else if (strcmp(input, "5") == 0)
        {
            while (1)
            {
                int parsed = 0;
                print_centered_header("UPDATE STOCK QUANTITY");
                printf("  Current : %d pcs\n", new_stock);
                printf("  New     [0-Back]: ");
                if (!fgets(input, sizeof(input), stdin))
                    return;
                trim_input(input);
                if (strcmp(input, "0") == 0)
                    break;
                if (sscanf(input, "%d", &parsed) != 1 || parsed <= 0)
                {
                    print_centered_header("!! INPUT ERROR");
                    printf("  Stock quantity cannot be 0 or negative.\n");
                    printf("  Please enter a valid stock quantity.\n");
                    printf("--------------------------------------------------------------------------------\n");
                    printf("  Enter Stock Quantity [0-Back]: ");
                    if (!fgets(input, sizeof(input), stdin))
                        return;
                    trim_input(input);
                    if (strcmp(input, "0") == 0)
                        break;
                    if (sscanf(input, "%d", &parsed) != 1 || parsed <= 0)
                        continue;
                }
                new_stock = parsed;
                break;
            }
        }
        else if (strcmp(input, "6") == 0)
        {
            while (1)
            {
                print_centered_header("UPDATE CONTACT");
                printf("  Current : %s\n", new_contact);
                printf("  Format  : 10 digits starting with 9 (e.g. 9857463274)\n");
                printf("  New     [0-Back]: ");
                if (!fgets(input, sizeof(input), stdin))
                    return;
                trim_input(input);
                if (strcmp(input, "0") == 0)
                    break;
                if (!validate_contact(input))
                {
                    print_centered_header("!! INPUT ERROR");
                    printf("  Enter exactly 10 digits starting with 9 (e.g. 9857463274).\n");
                    printf("  Please enter a valid mobile phone number.\n");
                    printf("--------------------------------------------------------------------------------\n");
                    printf("  Enter Contact [0-Back]: ");
                    if (!fgets(input, sizeof(input), stdin))
                        return;
                    trim_input(input);
                    if (strcmp(input, "0") == 0)
                        break;
                    if (!validate_contact(input))
                        continue;
                }
                strncpy(new_contact, input, sizeof(new_contact) - 1);
                new_contact[sizeof(new_contact) - 1] = '\0';
                break;
            }
        }
        else
        {
            continue;
        }

        print_centered_header("CONFIRM UPDATE");
        printf("  Please review your updated details.\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  Item ID      : %d\n", selected_item->id);
        printf("  Product Name : %s%s\n", new_title, strcmp(new_title, selected_item->title) != 0 ? "        (updated)" : "");
        printf("  Category     : %s > %s%s\n",
               new_category, new_title, strcmp(new_category, selected_item->category) != 0 ? "        (updated)" : "");
        printf("  Condition    : %s%s\n", new_condition, strcmp(new_condition, selected_item->condition) != 0 ? "        (updated)" : "");
        printf("  Price        : PHP %.2f%s\n", new_price, (new_price != selected_item->price) ? "        (updated)" : "");
        printf("  Stock        : %d pcs%s\n", new_stock, (new_stock != selected_item->quantity) ? "        (updated)" : "");
        printf("  Contact      : %s%s\n", new_contact, strcmp(new_contact, selected_item->contact) != 0 ? "        (updated)" : "");
        printf("================================================================================\n");
        printf("  [S] Save Changes   [E] Edit Again   [C] Cancel\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");
        if (!fgets(input, sizeof(input), stdin))
            return;
        trim_input(input);
        if (strlen(input) != 1)
            continue;

        if (toupper((unsigned char)input[0]) == 'E')
            continue;
        if (toupper((unsigned char)input[0]) == 'C')
            return;
        if (toupper((unsigned char)input[0]) != 'S')
            continue;

        strncpy(selected_item->title, new_title, sizeof(selected_item->title) - 1);
        selected_item->title[sizeof(selected_item->title) - 1] = '\0';
        strncpy(selected_item->category, new_category, sizeof(selected_item->category) - 1);
        selected_item->category[sizeof(selected_item->category) - 1] = '\0';
        strncpy(selected_item->condition, new_condition, sizeof(selected_item->condition) - 1);
        selected_item->condition[sizeof(selected_item->condition) - 1] = '\0';
        strncpy(selected_item->contact, new_contact, sizeof(selected_item->contact) - 1);
        selected_item->contact[sizeof(selected_item->contact) - 1] = '\0';
        selected_item->price = new_price;
        selected_item->quantity = new_stock;

        {
            char timestamp[64];
            get_timestamp(timestamp, sizeof(timestamp));
            print_centered_header("ITEM UPDATED!");
            printf("  Item successfully updated!\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Item ID      : %d\n", selected_item->id);
            printf("  Product Name : %s\n", selected_item->title);
            printf("  Price        : PHP %.2f\n", selected_item->price);
            printf("  Date Updated : %s\n", timestamp);
            printf("================================================================================\n");
            printf("  [V] View My Listings\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");
            if (!fgets(input, sizeof(input), stdin))
                return;
            trim_input(input);
            if (strlen(input) == 1 && toupper((unsigned char)input[0]) == 'V')
            {
                return;
            }
            return;
        }
    }
}

/* ============================================================
   SELLER — Incoming Orders (new STEP 1–6 UI)
   ============================================================ */

typedef enum
{
    ORDER_FILTER_ALL = 1,
    ORDER_FILTER_PENDING = 2,
    ORDER_FILTER_COMPLETED = 3,
    ORDER_FILTER_CANCELLED = 4
} OrderFilter;

typedef struct IncomingOrderRow
{
    int order_id;
    int item_id; /* representative item id (for display) */
    char item_title[100];
    char item_condition[30];
    char buyer[50];
    char buyer_contact[20];
    char payment_method[30];
    int quantity;
    float unit_price; /* representative unit price */
    float total;      /* aggregated total */
    char status[32];
    char date[20];
} IncomingOrderRow;

static int order_filter_matches(OrderFilter filter, const char *status)
{
    if (filter == ORDER_FILTER_ALL)
        return 1;
    if (!status)
        return 0;
    if (filter == ORDER_FILTER_PENDING && strcasecmp(status, "Pending") == 0)
        return 1;
    // Orders can be marked "Received" by the buyer; treat them as Completed for seller filtering.
    if (filter == ORDER_FILTER_COMPLETED &&
        (strcasecmp(status, "Completed") == 0 || strcasecmp(status, "Received") == 0))
        return 1;
    if (filter == ORDER_FILTER_CANCELLED && strcasecmp(status, "Cancelled") == 0)
        return 1;
    return 0;
}

static const char *seller_display_status(const char *status)
{
    if (!status)
        return "";
    // Hide intermediate "Received" from seller UI by showing it as Completed.
    if (strcasecmp(status, "Received") == 0)
        return "Completed";
    return status;
}

static int seller_matches(const char *seller, const char *seller_key1, const char *seller_key2)
{
    if (!seller)
        return 0;
    if (seller_key1 && strcasecmp(seller, seller_key1) == 0)
        return 1;
    if (seller_key2 && strcasecmp(seller, seller_key2) == 0)
        return 1;
    return 0;
}

static void mark_sold_silent(Catalog *catalog, int item_id)
{
    Item *it = get_item_by_id(catalog, item_id);
    if (it)
        it->is_sold = 1;
}

static void format_money_php(float amount, char *out, size_t out_size)
{
    // amount is assumed to be total price; we print PHP with comma separators.
    long long cents = (long long)(amount * 100.0 + (amount >= 0 ? 0.5 : -0.5));
    long long abs_cents = cents >= 0 ? cents : -cents;
    long long int_part = abs_cents / 100;
    int frac = (int)(abs_cents % 100);

    char int_str[64];
    // Avoid platform-specific printf modifiers (e.g., %lld).
    {
        unsigned long long u = (unsigned long long)int_part;
        if (u == 0)
        {
            int_str[0] = '0';
            int_str[1] = '\0';
        }
        else
        {
            char rev[64];
            int idx = 0;
            while (u > 0 && idx < (int)sizeof(rev) - 1)
            {
                rev[idx++] = (char)('0' + (u % 10ULL));
                u /= 10ULL;
            }
            rev[idx] = '\0';

            int len = idx;
            for (int i = 0; i < len; i++)
            {
                int_str[i] = rev[len - 1 - i];
            }
            int_str[len] = '\0';
        }
    }
    int len = (int)strlen(int_str);

    char with_commas[80];
    int pos = 0;
    for (int i = 0; i < len; i++)
    {
        if (i > 0 && ((len - i) % 3 == 0))
        {
            if (pos + 1 < (int)sizeof(with_commas))
                with_commas[pos++] = ',';
        }
        if (pos + 1 < (int)sizeof(with_commas))
            with_commas[pos++] = int_str[i];
    }
    with_commas[pos] = '\0';

    snprintf(out, out_size, "PHP %s.%02d", with_commas, frac);
}

static const char *qty_label(int qty)
{
    return qty == 1 ? "pc" : "pcs";
}

static void format_date_readable(const char *input, char *out, size_t out_size)
{
    // Convert "YYYY-MM-DD HH:MM" to "April 28, 2026  10:30 AM".
    int year = 0, month = 0, day = 0, hour = 0, minute = 0;
    if (!input || !out)
    {
        return;
    }

    if (sscanf(input, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) != 5)
    {
        snprintf(out, out_size, "%s", input);
        return;
    }

    static const char *months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"};
    const char *mon_name = (month >= 1 && month <= 12) ? months[month - 1] : "Unknown";

    const char *ampm = (hour >= 12) ? "PM" : "AM";
    int h12 = hour % 12;
    if (h12 == 0)
        h12 = 12;

    snprintf(out, out_size, "%s %d, %d  %d:%02d %s", mon_name, day, year, h12, minute, ampm);
}

static int collect_incoming_order_rows(TxStack *stack,
                                        const char *seller_key1,
                                        const char *seller_key2,
                                        OrderFilter filter,
                                        IncomingOrderRow rows[],
                                        int max_rows)
{
    int row_count = 0;

    for (TxNode *current = stack->top; current != NULL; current = current->next)
    {
        if (!seller_matches(current->seller, seller_key1, seller_key2))
            continue;
        if (!order_filter_matches(filter, current->status))
            continue;

        int existing_idx = -1;
        for (int i = 0; i < row_count; i++)
        {
            if (rows[i].order_id == current->order_id)
            {
                existing_idx = i;
                break;
            }
        }

        if (existing_idx == -1)
        {
            if (row_count >= max_rows)
                break;

            IncomingOrderRow *r = &rows[row_count];
            r->order_id = current->order_id;
            r->item_id = current->item_id;
            strncpy(r->item_title, current->item_title, sizeof(r->item_title) - 1);
            r->item_title[sizeof(r->item_title) - 1] = '\0';

            strncpy(r->item_condition, current->item_condition, sizeof(r->item_condition) - 1);
            r->item_condition[sizeof(r->item_condition) - 1] = '\0';

            strncpy(r->buyer, current->buyer, sizeof(r->buyer) - 1);
            r->buyer[sizeof(r->buyer) - 1] = '\0';

            strncpy(r->buyer_contact, current->buyer_contact, sizeof(r->buyer_contact) - 1);
            r->buyer_contact[sizeof(r->buyer_contact) - 1] = '\0';

            strncpy(r->payment_method, current->payment_method, sizeof(r->payment_method) - 1);
            r->payment_method[sizeof(r->payment_method) - 1] = '\0';

            r->quantity = current->quantity;
            r->unit_price = current->unit_price;
            r->total = current->price; /* total */

            strncpy(r->status, current->status, sizeof(r->status) - 1);
            r->status[sizeof(r->status) - 1] = '\0';

            strncpy(r->date, current->date, sizeof(r->date) - 1);
            r->date[sizeof(r->date) - 1] = '\0';

            row_count++;
        }
        else
        {
            rows[existing_idx].quantity += current->quantity;
            rows[existing_idx].total += current->price;
        }
    }

    return row_count;
}

static void update_order_status_for_seller(TxStack *stack,
                                            Catalog *catalog,
                                            const char *seller_key1,
                                            const char *seller_key2,
                                            int order_id,
                                            const char *new_status,
                                            int mark_item_sold)
{
    for (TxNode *current = stack->top; current != NULL; current = current->next)
    {
        if (!seller_matches(current->seller, seller_key1, seller_key2))
            continue;
        if (current->order_id != order_id)
            continue;

        strncpy(current->status, new_status, sizeof(current->status) - 1);
        current->status[sizeof(current->status) - 1] = '\0';

        if (mark_item_sold)
        {
    // Mark representative listing as sold to keep catalog consistent.
            mark_sold_silent(catalog, current->item_id);
        }
    }
}

static int read_int_line(void)
{
    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin))
        return 0;
    trim_input(buf);
    return atoi(buf);
}

static int read_choice_char(char *out_char)
{
    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin))
        return 0;
    trim_input(buf);
    if (buf[0] == '\0')
        return 0;
    *out_char = buf[0];
    return 1;
}

void seller_view_incoming_orders(Catalog *catalog, TxStack *stack, User *user)
{
    OrderFilter filter = ORDER_FILTER_ALL;
    const char *seller_key1 = user->display_name;
    const char *seller_key2 = user->username;

    while (1)
    {
        IncomingOrderRow rows[200];
        memset(rows, 0, sizeof(rows));

        int row_count = collect_incoming_order_rows(stack, seller_key1, seller_key2, filter, rows, 200);

    // --- STEP 6: No incoming orders ---
        if (row_count <= 0 && filter == ORDER_FILTER_ALL)
        {
            print_centered_header("INCOMING ORDERS");
            printf("  Logged in as: %s (Seller)\n", user->display_name);
            printf("--------------------------------------------------------------------------------\n");

            printf("\n                      No incoming orders yet.\n");
            printf("                 Your orders will appear here once\n");
            printf("                     buyers purchase your items.\n\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Enter choice [0-Back]: ");

            char ch;
            while (1)
            {
                if (!read_choice_char(&ch))
                    continue;
                ch = (char)toupper((unsigned char)ch);
                if (ch == 'B' || ch == '0')
                    return;
            }
        }

    // --- STEP 1 / STEP 5: Incoming orders list ---
        const char *showing_label = NULL;
        if (filter == ORDER_FILTER_PENDING)
        {
            showing_label = "Pending Orders";
        }
        else if (filter == ORDER_FILTER_COMPLETED)
        {
            showing_label = "Completed Orders";
        }
        else if (filter == ORDER_FILTER_CANCELLED)
        {
            showing_label = "Cancelled Orders";
        }

        if (filter == ORDER_FILTER_ALL)
        {
            print_centered_header("INCOMING ORDERS");
        }
        else
        {
    // Match requested title style.
            if (filter == ORDER_FILTER_PENDING)
                print_centered_header("INCOMING ORDERS (PENDING ONLY)");
            else if (filter == ORDER_FILTER_COMPLETED)
                print_centered_header("INCOMING ORDERS (COMPLETED ONLY)");
            else
                print_centered_header("INCOMING ORDERS (CANCELLED ONLY)");
        }

        printf("  Logged in as: %s (Seller)\n", user->display_name);
        printf("--------------------------------------------------------------------------------\n");

    // If the selected filter yields no rows, show the same STEP 6 empty state.
        if (row_count <= 0)
        {
            printf("\n                      No incoming orders yet.\n");
            printf("                 Your orders will appear here once\n");
            printf("                     buyers purchase your items.\n\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Enter choice [0-Back]: ");

            char ch;
            while (1)
            {
                if (!read_choice_char(&ch))
                    continue;
                ch = (char)toupper((unsigned char)ch);
                if (ch == 'B' || ch == '0')
                    return;
            }
        }

        printf("  No.  Order ID   Product Name     Buyer             Qty    Total        Status\n");
        printf("  ------------------------------------------------------------------------------\n");

    // Totals and status counts are only shown in STEP 1 (All Orders).
        int pending_count = 0, completed_count = 0, cancelled_count = 0;
        if (filter == ORDER_FILTER_ALL)
        {
            IncomingOrderRow all_rows[200];
            memset(all_rows, 0, sizeof(all_rows));
            int all_count = collect_incoming_order_rows(stack, seller_key1, seller_key2, ORDER_FILTER_ALL, all_rows, 200);
            for (int i = 0; i < all_count; i++)
            {
                if (strcasecmp(all_rows[i].status, "Pending") == 0)
                    pending_count++;
                else if (strcasecmp(all_rows[i].status, "Completed") == 0 ||
                         strcasecmp(all_rows[i].status, "Received") == 0)
                    completed_count++;
                else if (strcasecmp(all_rows[i].status, "Cancelled") == 0)
                    cancelled_count++;
            }
        }

        for (int i = 0; i < row_count; i++)
        {
            char total_buf[64];
            format_money_php(rows[i].total, total_buf, sizeof(total_buf));
            printf("  [%2d]  #%05d     %-15s %-17s %d %-4s  %-11s %-10s\n",
                   i + 1,
                   rows[i].order_id,
                   rows[i].item_title,
                   rows[i].buyer,
                   rows[i].quantity,
                   qty_label(rows[i].quantity),
                   total_buf,
                   seller_display_status(rows[i].status));
        }

        printf("  ------------------------------------------------------------------------------\n");

        if (filter == ORDER_FILTER_ALL)
        {
            int total_orders = row_count;
            printf("  Pending   : %d order(s)\n", pending_count);
            printf("  Completed : %d order(s)\n", completed_count);
            printf("  Cancelled : %d order(s)\n", cancelled_count);
            printf("  Total     : %d order(s)\n", total_orders);
            printf("================================================================================\n");
            printf("  [V] View Order   [A] Accept Order   [D] Decline Order\n");
            printf("================================================================================\n");
        }
        else
        {
            char showing_buf[64];
            snprintf(showing_buf, sizeof(showing_buf), "%s", showing_label ? showing_label : "Orders");
            printf("================================================================================\n");
            printf("  Showing: %s          %d order(s) found\n", showing_buf, row_count);
            printf("================================================================================\n");
            printf("  [V] View Order   [A] Accept Order   [D] Decline Order   [F] Filter\n");
            printf("================================================================================\n");
        }

        printf("  Enter choice [0-Back]: ");

        char choice = 0;
        if (!read_choice_char(&choice))
            continue;
        choice = (char)toupper((unsigned char)choice);

        if (choice == 'B' || choice == '0')
        {
            return;
        }
        else if (choice == 'F')
        {
    // STEP 5 filter menu
            print_centered_header("INCOMING ORDERS");
            printf("  Filter by Status:\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  [1] All Orders\n");
            printf("  [2] Pending Only\n");
            printf("  [3] Completed Only\n");
            printf("  [4] Cancelled Only\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char fch = 0;
            if (!read_choice_char(&fch))
                continue;
            fch = (char)toupper((unsigned char)fch);
            if (fch == 'B' || fch == '0')
            {
                continue; /* Back to incoming orders list */
            }
            else if (fch == '1')
                filter = ORDER_FILTER_ALL;
            else if (fch == '2')
                filter = ORDER_FILTER_PENDING;
            else if (fch == '3')
                filter = ORDER_FILTER_COMPLETED;
            else if (fch == '4')
                filter = ORDER_FILTER_CANCELLED;
            else
                filter = ORDER_FILTER_ALL;
            continue;
        }
        else if (choice == 'V')
        {
            print_centered_header("INCOMING ORDERS");
            printf("  Enter No. to View [0-Back]: ");
            int sel = read_int_line();
            if (sel <= 0)
                continue;
            if (sel > row_count)
                continue;

            IncomingOrderRow *picked = &rows[sel - 1];

    // --- STEP 2: View order detail ---
            char date_buf[64];
            format_date_readable(picked->date, date_buf, sizeof(date_buf));

            {
                char order_title[64];
                snprintf(order_title, sizeof(order_title), "ORDER DETAILS #%05d", picked->order_id);
                print_centered_header(order_title);
            }
            printf("  Order ID     : #%05d\n", picked->order_id);
            printf("  Date Ordered : %s\n", date_buf);
            printf("  -------------------------\n");
            printf("  Product Name : %s\n", picked->item_title);
            printf("  Condition    : %s\n", picked->item_condition);

            printf("  Unit Price   : ");
            char unit_buf[64];
            format_money_php(picked->unit_price, unit_buf, sizeof(unit_buf));
            printf("%s\n", unit_buf);

            printf("  Quantity     : %d %s\n", picked->quantity, qty_label(picked->quantity));
            printf("  Total        : ");
            char total_buf[64];
            format_money_php(picked->total, total_buf, sizeof(total_buf));
            printf("%s\n", total_buf);

            printf("  -------------------------\n");
            printf("  Buyer        : %s\n", picked->buyer);
            printf("  Contact      : %s\n", picked->buyer_contact);
            printf("  Payment      : %s\n", picked->payment_method);
                    printf("  Status       : %s\n", seller_display_status(picked->status));

            printf("================================================================================\n");
            printf("  [A] Accept Order   [D] Decline Order\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char detail_choice = 0;
            if (!read_choice_char(&detail_choice))
                continue;
            detail_choice = (char)toupper((unsigned char)detail_choice);

            if (detail_choice == 'B' || detail_choice == '0')
            {
                continue;
            }
            else if (detail_choice == 'A' || detail_choice == 'D')
            {
                if (strcasecmp(picked->status, "Pending") != 0)
                {
    // --- STEP 4: Cannot process ---
                    printf("================================================================================\n");
                    printf("  !! CANNOT PROCESS ORDER\n");
                    printf("================================================================================\n");
                            printf("  Order #%05d is already %s.\n",
                                   picked->order_id,
                                   seller_display_status(picked->status));
                    printf("  Only PENDING orders can be accepted or declined.\n");
                    printf("================================================================================\n");
                    printf("  Enter choice [0-Back]: ");
                    while (1)
                    {
                        char backch;
                        if (!read_choice_char(&backch))
                            continue;
                        backch = (char)toupper((unsigned char)backch);
                        if (backch == 'B' || backch == '0')
                            break;
                    }
                    continue;
                }

    // --- STEP 3: Confirm accept/decline ---
                const int is_accept = (detail_choice == 'A');
                const char *target_status = is_accept ? "Awaiting Confirmation" : "Cancelled";

                printf("================================================================================\n");
                printf("                               %s ORDER\n", is_accept ? "ACCEPT" : "DECLINE");
                printf("================================================================================\n");
                printf("  Are you sure you want to %s this order?\n", is_accept ? "accept" : "decline");
                printf("--------------------------------------------------------------------------------\n");
                printf("  Order ID   : #%05d\n", picked->order_id);
                printf("  Item       : %s\n", picked->item_title);
                printf("  Buyer      : %s\n", picked->buyer);
                printf("  Total      : %s\n", total_buf);
                printf("  Payment    : %s\n", picked->payment_method);
                printf("================================================================================\n");
                printf("  [Y] Yes, %s   [N] No, Go Back\n", is_accept ? "Accept" : "Decline");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char conf = 0;
                if (!read_choice_char(&conf))
                    continue;
                conf = (char)toupper((unsigned char)conf);

                if (conf != 'Y')
                    continue;

                update_order_status_for_seller(stack, catalog, seller_key1, seller_key2, picked->order_id, target_status, is_accept);

    // Update picked for screen rendering.
                strncpy(picked->status, target_status, sizeof(picked->status) - 1);
                picked->status[sizeof(picked->status) - 1] = '\0';

    // --- Post-confirm success screen ---
                printf("================================================================================\n");
                printf("                               %s ORDER\n", is_accept ? "ACCEPT" : "DECLINE");
                printf("================================================================================\n");
                printf("  %s Order #%05d has been %s!\n",
                       is_accept ? "✔" : "✔",
                       picked->order_id,
                       is_accept ? "accepted" : "declined");
                printf("--------------------------------------------------------------------------------\n");
                printf("  Item       : %s\n", picked->item_title);
                printf("  Buyer      : %s\n", picked->buyer);
                printf("  Total      : %s\n", total_buf);
                printf("  Status     : %s\n", seller_display_status(picked->status));
                printf("--------------------------------------------------------------------------------\n");
                printf("  Buyer Contact : %s\n", picked->buyer_contact);
                if (is_accept)
                {
                    printf("  Please coordinate with the buyer for delivery.\n");
                }
                else
                {
                    printf("  This order was not fulfilled.\n");
                }
                printf("================================================================================\n");
                printf("  [V] View Incoming Orders\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char post_choice = 0;
                if (!read_choice_char(&post_choice))
                    continue;
                post_choice = (char)toupper((unsigned char)post_choice);
                if (post_choice == 'B' || post_choice == '0')
                    return;
    // V: go back to list (reset to All for a clean view like sample).
                filter = ORDER_FILTER_ALL;
                continue;
            }

            continue;
        }
        else if (choice == 'A' || choice == 'D')
        {
    // Accept/Decline from list (not shown in sample flow, but required by menu keys).
            printf("  Enter No. to %s [0-Back]: ", choice == 'A' ? "Accept" : "Decline");
            int sel = read_int_line();
            if (sel <= 0 || sel > row_count)
                continue;
            IncomingOrderRow *picked = &rows[sel - 1];

            if (strcasecmp(picked->status, "Pending") != 0)
            {
                printf("================================================================================\n");
                printf("  !! CANNOT PROCESS ORDER\n");
                printf("================================================================================\n");
                printf("  Order #%05d is already %s.\n", picked->order_id, seller_display_status(picked->status));
                printf("  Only PENDING orders can be accepted or declined.\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");
                while (1)
                {
                    char backch;
                    if (!read_choice_char(&backch))
                        continue;
                    backch = (char)toupper((unsigned char)backch);
                    if (backch == 'B' || backch == '0')
                        break;
                }
                continue;
            }

            int is_accept = (choice == 'A');
            const char *target_status = is_accept ? "Awaiting Confirmation" : "Cancelled";
            char total_buf[64];
            format_money_php(picked->total, total_buf, sizeof(total_buf));

            printf("================================================================================\n");
            printf("                               %s ORDER\n", is_accept ? "ACCEPT" : "DECLINE");
            printf("================================================================================\n");
            printf("  Are you sure you want to %s this order?\n", is_accept ? "accept" : "decline");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Order ID   : #%05d\n", picked->order_id);
            printf("  Item       : %s\n", picked->item_title);
            printf("  Buyer      : %s\n", picked->buyer);
            printf("  Total      : %s\n", total_buf);
            printf("  Payment    : %s\n", picked->payment_method);
            printf("================================================================================\n");
            printf("  [Y] Yes, %s   [N] No, Go Back\n", is_accept ? "Accept" : "Decline");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char conf = 0;
            if (!read_choice_char(&conf))
                continue;
            conf = (char)toupper((unsigned char)conf);
            if (conf != 'Y')
                continue;

            update_order_status_for_seller(stack, catalog, seller_key1, seller_key2, picked->order_id, target_status, is_accept);
            strncpy(picked->status, target_status, sizeof(picked->status) - 1);
            picked->status[sizeof(picked->status) - 1] = '\0';

            printf("================================================================================\n");
            printf("                               %s ORDER\n", is_accept ? "ACCEPT" : "DECLINE");
            printf("================================================================================\n");
            printf("  ✔ Order #%05d has been %s!\n",
                   picked->order_id,
                   is_accept ? "accepted" : "declined");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Item       : %s\n", picked->item_title);
            printf("  Buyer      : %s\n", picked->buyer);
            printf("  Total      : %s\n", total_buf);
            printf("  Status     : %s\n", seller_display_status(picked->status));
            printf("--------------------------------------------------------------------------------\n");
            printf("  Buyer Contact : %s\n", picked->buyer_contact);
            if (is_accept)
                printf("  Please coordinate with the buyer for delivery.\n");
            else
                printf("  This order was not fulfilled.\n");
            printf("================================================================================\n");
            printf("  [V] View Incoming Orders\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char post_choice = 0;
            if (!read_choice_char(&post_choice))
                continue;
            post_choice = (char)toupper((unsigned char)post_choice);
            if (post_choice == 'B' || post_choice == '0')
                return;
            filter = ORDER_FILTER_ALL;
            continue;
        }

        continue;
    }
}

/* ============================================================
   SELLER — Sales History (matches user's provided layout)
   ============================================================ */

typedef struct SalesOrderRow
{
    int order_id;
    char item_title[100];
    char item_condition[30];
    char buyer[50];
    char buyer_contact[20];
    char payment_method[30];
    int quantity;
    float unit_price;
    float total;
    char status[32]; /* Awaiting Confirmation | Completed | Cancelled */
    char date[20];
} SalesOrderRow;

static const char *sales_display_status(const char *status)
{
    if (!status)
        return "";
    if (strcasecmp(status, "Received") == 0)
        return "Completed";
    return status;
}

void seller_sales_history(TxStack *stack, User *user)
{
    while (1)
    {
        SalesOrderRow rows[200];
        memset(rows, 0, sizeof(rows));
        int row_count = 0;

        float total_earned = 0.0f;
        float total_missed = 0.0f;
        int completed_orders = 0;
        int cancelled_orders = 0;

        for (TxNode *current = stack->top; current != NULL; current = current->next)
        {
            if (!seller_matches(current->seller, user->display_name, user->username))
                continue;

            if (strcasecmp(current->status, "Pending") == 0)
                continue; /* not part of sales history */

            const char *disp_status = sales_display_status(current->status);
            if (strcasecmp(disp_status, "Pending") == 0)
                continue;

            int idx = -1;
            for (int i = 0; i < row_count; i++)
            {
                if (rows[i].order_id == current->order_id)
                {
                    idx = i;
                    break;
                }
            }

            if (idx < 0)
            {
                if (row_count >= 200)
                    break;

                SalesOrderRow *r = &rows[row_count];
                r->order_id = current->order_id;
                strncpy(r->item_title, current->item_title, sizeof(r->item_title) - 1);
                r->item_title[sizeof(r->item_title) - 1] = '\0';

                strncpy(r->item_condition, current->item_condition, sizeof(r->item_condition) - 1);
                r->item_condition[sizeof(r->item_condition) - 1] = '\0';

                strncpy(r->buyer, current->buyer, sizeof(r->buyer) - 1);
                r->buyer[sizeof(r->buyer) - 1] = '\0';

                strncpy(r->buyer_contact, current->buyer_contact, sizeof(r->buyer_contact) - 1);
                r->buyer_contact[sizeof(r->buyer_contact) - 1] = '\0';

                strncpy(r->payment_method, current->payment_method, sizeof(r->payment_method) - 1);
                r->payment_method[sizeof(r->payment_method) - 1] = '\0';

                r->quantity = current->quantity;
                r->unit_price = current->unit_price;
                r->total = current->price;

                strncpy(r->status, disp_status, sizeof(r->status) - 1);
                r->status[sizeof(r->status) - 1] = '\0';

                strncpy(r->date, current->date, sizeof(r->date) - 1);
                r->date[sizeof(r->date) - 1] = '\0';

                row_count++;
            }
            else
            {
                rows[idx].quantity += current->quantity;
                rows[idx].total += current->price;
    // Keep first item/condition/unit price for display simplicity.
            }
        }

        for (int i = 0; i < row_count; i++)
        {
            if (strcasecmp(rows[i].status, "Completed") == 0)
            {
                completed_orders++;
                total_earned += rows[i].total;
            }
            else if (strcasecmp(rows[i].status, "Cancelled") == 0)
            {
                cancelled_orders++;
                total_missed += rows[i].total;
            }
        }

    // --- No sales history ---
        if (row_count <= 0)
        {
            print_centered_header("SALES HISTORY");
            printf("  Logged in as: %s (Seller)\n", user->display_name);
            printf("--------------------------------------------------------------------------------\n");
            printf("\n                       No sales history yet.\n");
            printf("                  Start selling to see your history!\n\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("  Enter choice [0-Back]: ");

            char backch[32];
            if (!fgets(backch, sizeof(backch), stdin))
                return;
            trim_input(backch);
            if (strcasecmp(backch, "B") == 0 || strcmp(backch, "0") == 0)
                return;
            continue;
        }

    // --- Sales history list ---
        char earned_buf[64];
        char missed_buf[64];
        format_money_php(total_earned, earned_buf, sizeof(earned_buf));
        format_money_php(total_missed, missed_buf, sizeof(missed_buf));

        print_centered_header("SALES HISTORY");
        printf("  Logged in as: %s (Seller)\n", user->display_name);
        printf("--------------------------------------------------------------------------------\n");
        printf("  No.  Order ID   Product Name     Buyer             Total        Status\n");
        printf("  ------------------------------------------------------------------------------\n");

        for (int i = 0; i < row_count; i++)
        {
            char total_buf[64];
            format_money_php(rows[i].total, total_buf, sizeof(total_buf));
            printf("  [%2d]  #%05d     %-15s %-17s %-15s %-10s\n",
                   i + 1,
                   rows[i].order_id,
                   rows[i].item_title,
                   rows[i].buyer,
                   total_buf,
                   rows[i].status);
        }

        printf("  ------------------------------------------------------------------------------\n");
        printf("  Completed : %d order(s)    Total Earned : %s\n", completed_orders, earned_buf);
        printf("  Cancelled : %d order(s)    Total Missed : %s\n", cancelled_orders, missed_buf);
        printf("  Total     : %d order(s)\n", row_count);
        printf("================================================================================\n");
        printf("  Enter No. to View Details [0-Back]: ");

        char input[32];
        if (!fgets(input, sizeof(input), stdin))
            return;
        trim_input(input);

        int choice = atoi(input);
        if (choice == 0)
            return;
        if (choice < 1 || choice > row_count)
            continue;

        SalesOrderRow *picked = &rows[choice - 1];

    // --- Sale details screen ---
        char date_buf[64];
        format_date_readable(picked->date, date_buf, sizeof(date_buf));

        char unit_buf[64];
        char total_buf[64];
        format_money_php(picked->unit_price, unit_buf, sizeof(unit_buf));
        format_money_php(picked->total, total_buf, sizeof(total_buf));

        {
            char sale_title[64];
            snprintf(sale_title, sizeof(sale_title), "SALE DETAILS #%05d", picked->order_id);
            print_centered_header(sale_title);
        }
        printf("  Order ID     : #%05d\n", picked->order_id);
        printf("  Date         : %s\n", date_buf);
        printf("  -------------------------\n");
        printf("  Product Name : %s\n", picked->item_title);
        printf("  Condition    : %s\n", picked->item_condition);
        printf("  Unit Price   : %s\n", unit_buf);
        printf("  Quantity     : %d %s\n", picked->quantity, qty_label(picked->quantity));
        printf("  Total        : %s\n", total_buf);
        printf("  -------------------------\n");
        printf("  Buyer        : %s\n", picked->buyer);
        printf("  Contact      : %s\n", picked->buyer_contact);
        printf("  Payment      : %s\n", picked->payment_method);
        printf("  Status       : %s\n", picked->status);
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        char back[32];
        if (!fgets(back, sizeof(back), stdin))
            return;
        trim_input(back);
        if (strcasecmp(back, "B") == 0 || strcmp(back, "0") == 0)
            continue;
    }
}

    // --- Count pending orders for an item ---
int count_pending_orders_for_item(InquiryQueue *q, int item_id)
{
    int count = 0;
    InquiryNode *current = q->front;
    while (current != NULL)
    {
        if (current->item_id == item_id)
        {
            count++;
        }
        current = current->next;
    }
    return count;
}

    // PH mobile: local format with exactly 10 digits starting with 9 (e.g. 9857463274)
static int validate_contact(const char *contact)
{
    if (!contact || !*contact)
        return 0;

    char buf[128];
    strncpy(buf, contact, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_input(buf);

    if ((int)strlen(buf) != 10)
        return 0;

    if (buf[0] != '9')
        return 0;

    for (int i = 0; i < 10; i++)
    {
        if (!isdigit((unsigned char)buf[i]))
            return 0;
    }
    return 1;
}

static int validate_password(const char *password)
{
    /* Password requirements:
       - Minimum 6 characters
       - At least one letter (A-Z, a-z)
       - At least one number (0-9)
    */
    if (!password)
        return 0;

    int len = strlen(password);
    if (len < 6)
        return 0; /* Too short */

    int has_letter = 0;
    int has_number = 0;

    for (int i = 0; i < len; i++)
    {
        if (isalpha((unsigned char)password[i]))
            has_letter = 1;
        else if (isdigit((unsigned char)password[i]))
            has_number = 1;
    }

    return (has_letter && has_number);
}

static int validate_username(const char *username)
{
    /* Username requirements (same as password):
       - Minimum 6 characters
       - At least one letter (A-Z, a-z)
       - At least one number (0-9)
    */
    if (!username)
        return 0;

    int len = strlen(username);
    if (len < 6)
        return 0; /* Too short */

    int has_letter = 0;
    int has_number = 0;

    for (int i = 0; i < len; i++)
    {
        if (isalpha((unsigned char)username[i]))
            has_letter = 1;
        else if (isdigit((unsigned char)username[i]))
            has_number = 1;
    }

    return (has_letter && has_number);
}

    // Check for duplicate item with same title by same seller
static int check_duplicate_item(Catalog *catalog, const char *title, const char *seller)
{
    Item *current = catalog->head;
    while (current != NULL)
    {
        if (strcasecmp(current->title, title) == 0 &&
            strcasecmp(current->seller, seller) == 0)
        {
            return current->id; /* Return the ID of the duplicate */
        }
        current = current->next;
    }
    return -1; /* No duplicate found */
}

    // Step-style header for the add product workflow
static void print_add_product_header(const User *user, int step, const char *step_label)
{
    print_centered_header("ADD PRODUCT");
    printf("  Logged in as: %s (Seller)\n", user->display_name);
    printf("\n");
}

    // Display item details for confirmation
static void display_item_details_for_confirmation(const char *title, const char *category,
                                                   const char *subcategory, const char *condition,
                                                   float price, int stock, const char *seller,
                                                   const char *contact, int contact_type)
{
    print_centered_header("CONFIRM LISTING");
    printf("  Please review your product details before posting.\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("  Product Name : %s\n", title);
    printf("  Category     : %s > %s\n", category, subcategory);
    printf("  Condition    : %s\n", condition);
    printf("  Price        : PHP %.2f\n", price);
    printf("  Stock        : %d pcs\n", stock);
    printf("  Seller       : %s\n", seller);
    if (contact_type == 1)
        printf("  Contact      : %s (Phone Number)\n", contact);
    else if (contact_type == 2)
        printf("  Contact      : %s (Messenger)\n", contact);
    else
        printf("  Contact      : %s\n", contact);
    printf("================================================================================\n");
}

    // Main step-by-step add product function
void seller_add_item_step_by_step(Catalog *catalog, InquiryQueue *q, HashTable *ht, User *user)
{
    char title[100] = {0};
    char category[50] = {0};
    char subcategory[50] = {0};
    char condition[20] = {0};
    float price = 0.0;
    int stock = 0;
    char seller_name[50] = {0};
    char contact[64] = {0};
    int contact_type = 0; /* 1=Phone, 2=Messenger */

    strcpy(seller_name, user->display_name);

    // Product name
    while (1)
    {
        print_add_product_header(user, 1, "PRODUCT NAME");
        printf("  PRODUCT NAME\n");
        printf("================================================================================\n");
        printf("  Enter Product Name [0-Back]: ");
        fgets(title, sizeof(title), stdin);
        title[strcspn(title, "\n")] = 0;

        if (strcmp(title, "0") == 0)
            return;
        if (strlen(title) == 0)
        {
            printf("  [ERROR] Product name cannot be empty.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        break;
    }

    // Category
    while (1)
    {
        print_add_product_header(user, 2, "SELECT CATEGORY");
        printf("  Product Name  : %s\n", title);
        printf("\n");
        printf("  SELECT CATEGORY\n");
        printf("================================================================================\n");
        printf("  [1]  Uniform\n");
        printf("  [2]  Supply\n");
        printf("  [3]  Book\n");
        printf("  [4]  Other\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        int cat_choice;
        if (scanf("%d", &cat_choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        clear_input_buffer();

        if (cat_choice == 0)
            return;

        switch (cat_choice)
        {
        case 1:
            strcpy(category, "Uniform");
            break;
        case 2:
            strcpy(category, "Supply");
            break;
        case 3:
            strcpy(category, "Book");
            break;
        case 4:
            strcpy(category, "Other");
            break;
        default:
            printf("  [ERROR] Invalid choice.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        break;
    }

    // Other: skip sub-category — go straight to condition; listing uses category "other" + your title
    if (strcmp(category, "Other") == 0)
    {
        strncpy(subcategory, title, sizeof(subcategory) - 1);
        subcategory[sizeof(subcategory) - 1] = '\0';
        goto add_product_after_subcategory;
    }

    // Sub-category
    while (1)
    {
        print_add_product_header(user, 3, "SELECT SUB-CATEGORY");
        printf("  Product Name  : %s\n", title);
        printf("  Category      : %s\n", category);
        printf("\n");
        printf("  SELECT SUB-CATEGORY\n");
        printf("================================================================================\n");

        int subcat_choice;
        
    // Display subcategories based on main category
        if (strcmp(category, "Uniform") == 0)
        {
            printf("  [1]  SCHOOL UNIFORM\n");
            printf("  [2]  ROTC UNIFORM\n");
            printf("  [3]  PATHFIT UNIFORM\n");
        }
        else if (strcmp(category, "Supply") == 0)
        {
            printf("  [1]  Notebooks\n");
            printf("  [2]  Pens\n");
            printf("  [3]  Art Materials\n");
            printf("  [4]  Others\n");
        }
        else if (strcmp(category, "Book") == 0)
        {
            printf("  [1]  Textbooks\n");
            printf("  [2]  Reviewers\n");
            printf("  [3]  Novels\n");
            printf("  [4]  Others\n");
        }
        else if (strcmp(category, "Other") == 0)
        {
            printf("  [1]  Miscellaneous items\n");
            printf("  [2]  Others\n");
        }
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");
        if (scanf("%d", &subcat_choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        clear_input_buffer();

        if (subcat_choice == 0)
            return;

        if (strcmp(category, "Uniform") == 0)
        {
            switch (subcat_choice)
            {
            case 1:
                strcpy(subcategory, "SCHOOL UNIFORM");
                break;
            case 2:
                strcpy(subcategory, "ROTC UNIFORM");
                break;
            case 3:
                strcpy(subcategory, "PATHFIT UNIFORM");
                break;
            default:
                printf("  [ERROR] Invalid choice.\n");
                printf("  Press any key to continue...\n");
                getchar();
                continue;
            }
        }
        else if (strcmp(category, "Supply") == 0)
        {
            switch (subcat_choice)
            {
            case 1:
                strcpy(subcategory, "Notebooks");
                break;
            case 2:
                strcpy(subcategory, "Pens");
                break;
            case 3:
                strcpy(subcategory, "Art Materials");
                break;
            case 4:
                strcpy(subcategory, "Others");
                break;
            default:
                printf("  [ERROR] Invalid choice.\n");
                printf("  Press any key to continue...\n");
                getchar();
                continue;
            }
        }
        else if (strcmp(category, "Book") == 0)
        {
            switch (subcat_choice)
            {
            case 1:
                strcpy(subcategory, "Textbooks");
                break;
            case 2:
                strcpy(subcategory, "Reviewers");
                break;
            case 3:
                strcpy(subcategory, "Novels");
                break;
            case 4:
                strcpy(subcategory, "Others");
                break;
            default:
                printf("  [ERROR] Invalid choice.\n");
                printf("  Press any key to continue...\n");
                getchar();
                continue;
            }
        }
        break;
    }

add_product_after_subcategory:

    // Condition
    while (1)
    {
        print_add_product_header(user, 4, "SELECT CONDITION");
        printf("  Product Name  : %s\n", title);
        printf("  Category      : %s > %s\n", category, subcategory);
        printf("\n");
        printf("  SELECT CONDITION\n");
        printf("================================================================================\n");
        printf("  [1]  Brand New\n");
        printf("  [2]  Good Condition\n");
        printf("  [3]  Worn\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        int cond_choice;
        if (scanf("%d", &cond_choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        clear_input_buffer();

        if (cond_choice == 0)
            return;

        switch (cond_choice)
        {
        case 1:
            strcpy(condition, "Brand New");
            break;
        case 2:
            strcpy(condition, "Good Condition");
            break;
        case 3:
            strcpy(condition, "Worn");
            break;
        default:
            printf("  [ERROR] Invalid choice.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        break;
    }

    // Price
    while (1)
    {
        print_add_product_header(user, 5, "ENTER PRICE");
        printf("  Product Name  : %s\n", title);
        printf("  Category      : %s > %s\n", category, subcategory);
        printf("  Condition     : %s\n", condition);
        printf("\n");
        printf("  ENTER PRICE\n");
        printf("================================================================================\n");
        printf("  Enter Price (PHP) [0-Back]: ");
        if (scanf("%f", &price) != 1)
        {
            clear_input_buffer();
            printf("  !!INPUT ERROR\n");
            printf("================================================================================\n");
            printf("  Please enter a valid price.\n");
            printf("--------------------------------------------------------------------------------\n");
            continue;
        }
        clear_input_buffer();

        if (price == 0)
            return;

        if (price <= 0)
        {
            printf("  !!INPUT ERROR\n");
            printf("================================================================================\n");
            printf("  Price cannot be 0 or negative.\n");
            printf("  Please enter a valid price.\n");
            printf("--------------------------------------------------------------------------------\n");
            continue;
        }
        break;
    }

    // Stock
    while (1)
    {
        print_add_product_header(user, 6, "ENTER STOCK QUANTITY");
        printf("  Product Name  : %s\n", title);
        printf("  Category      : %s > %s\n", category, subcategory);
        printf("  Condition     : %s\n", condition);
        printf("  Price         : PHP %.2f\n", price);
        printf("\n");
        printf("  ENTER STOCK QUANTITY\n");
        printf("================================================================================\n");
        printf("  Enter Stock Quantity [0-Back]: ");
        if (scanf("%d", &stock) != 1)
        {
            clear_input_buffer();
            printf("  !!INPUT ERROR\n");
            printf("================================================================================\n");
            printf("  Please enter a valid stock quantity.\n");
            printf("--------------------------------------------------------------------------------\n");
            continue;
        }
        clear_input_buffer();

        if (stock == 0)
            return;

        if (stock <= 0)
        {
            printf("  !!INPUT ERROR\n");
            printf("================================================================================\n");
            printf("  Stock quantity cannot be 0 or negative.\n");
            printf("  Please enter a valid stock quantity.\n");
            printf("--------------------------------------------------------------------------------\n");
            continue;
        }
        break;
    }

    // Seller Name
    print_add_product_header(user, 7, "SELLER NAME");
    printf("  Product Name  : %s\n", title);
    printf("  Category      : %s > %s\n", category, subcategory);
    printf("  Condition     : %s\n", condition);
    printf("  Price         : PHP %.2f\n", price);
    printf("  Stock         : %d pcs\n", stock);
    printf("\n");
    printf("  SELLER NAME\n");
    printf("================================================================================\n");
    printf("  Seller Name   : %s\n", seller_name);
    printf("================================================================================\n");
    printf("\n");

    // Contact
    while (1)
    {
        print_add_product_header(user, 8, "CONTACT INFO");
        printf("  Product Name  : %s\n", title);
        printf("  Category      : %s > %s\n", category, subcategory);
        printf("  Condition     : %s\n", condition);
        printf("  Price         : PHP %.2f\n", price);
        printf("  Stock         : %d pcs\n", stock);
        printf("  Seller        : %s\n", seller_name);
        printf("\n");
        printf("  CONTACT INFO\n");
        printf("================================================================================\n");
        printf("  [1]  Phone Number\n");
        printf("  [2]  Messenger Name\n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        int contact_choice;
        if (scanf("%d", &contact_choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input. Please enter 1, 2, or 0.\n");
            printf("  Press any key to continue...\n");
            getchar();
            continue;
        }
        clear_input_buffer();

        if (contact_choice == 0)
            return;

        if (contact_choice == 1)
        {
            while (1)
            {
                printf("  Enter Phone Number [0-Back]: ");
                {
                    char temp[16] = {0};
                    if (!fgets(temp, sizeof(temp), stdin))
                        return;
                    if (strchr(temp, '\n') == NULL)
                    {
                        int c;
                        while ((c = getchar()) != '\n' && c != EOF);
                    }
                    temp[strcspn(temp, "\n")] = 0;
                    strncpy(contact, temp, sizeof(contact) - 1);
                    contact[sizeof(contact) - 1] = '\0';
                }

                if (strcmp(contact, "0") == 0)
                    break;

                if (!validate_contact(contact))
                {
                    printf("  !!INPUT ERROR\n");
                    printf("================================================================================\n");
                    printf("  Enter exactly 10 digits starting with 9 (e.g. 9857463274).\n");
                    printf("  Please enter a valid mobile phone number.\n");
                    printf("--------------------------------------------------------------------------------\n");
                    continue;
                }

                contact_type = 1;
                break;
            }
        }
        else if (contact_choice == 2)
        {
            while (1)
            {
                printf("  Enter Messenger Name [0-Back]: ");
                fgets(contact, sizeof(contact), stdin);
                contact[strcspn(contact, "\n")] = 0;

                if (strcmp(contact, "0") == 0)
                    break;

                if (strlen(contact) == 0)
                {
                    printf("  [ERROR] Messenger name cannot be empty.\n");
                    continue;
                }

                contact_type = 2;
                break;
            }
        }
        else
        {
            printf("  [ERROR] Invalid choice. Please enter 1, 2, or 0.\n");
            continue;
        }

        if (contact_type != 0)
            break;
    }

    // Check for duplicate
    int duplicate_id = check_duplicate_item(catalog, title, user->username);
    if (duplicate_id != -1)
    {
        Item *dup_item = get_item_by_id(catalog, duplicate_id);
        printf("\n================================================================================\n");
        printf("  !!WARNING — Similar Listing Found\n");
        printf("================================================================================\n");
        printf("  You already have a similar listing:\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("  ID      Product Name Condition     Price        Status\n");
        printf("  %-7d %-12s %-13s PHP %-9.2f %s\n", dup_item->id, dup_item->title,
               dup_item->condition, dup_item->price, dup_item->is_sold ? "Sold" : "Active");
        printf("--------------------------------------------------------------------------------\n");
        printf("  Do you still want to add a new listing?\n");
        printf("                    [Y] Yes, Continue   [N] No, Go Back                         \n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        char choice;
        if (scanf(" %c", &choice) != 1)
        {
            clear_input_buffer();
            return;
        }
        clear_input_buffer();

        if (toupper((unsigned char)choice) != 'Y')
            return;
    }

    // Confirmation screen with edit option
    while (1)
    {
        printf("\n");
        display_item_details_for_confirmation(title, category, subcategory, condition, price, stock,
                                               seller_name, contact, contact_type);
        printf("                    [P] Post Item   [E] Edit Details   [C] Cancel               \n");
        printf("================================================================================\n");
        printf("  Enter choice [0-Back]: ");

        char choice;
        if (scanf(" %c", &choice) != 1)
        {
            clear_input_buffer();
            printf("  [ERROR] Invalid input.\n");
            continue;
        }
        clear_input_buffer();
        choice = toupper((unsigned char)choice);

        if (choice == 'C')
        {
            print_centered_header("ADD PRODUCT CANCELLED");
            printf("  Logged in as: %s (Seller)\n", user->display_name);
            printf("\n");
            printf("  No item was added to your listings.\n");
            printf("\n");
            printf("================================================================================\n");
            printf("  Press any key to go back...");
            getchar();
            return;
        }

        if (choice == 'E')
        {
    // Edit Details
            while (1)
            {
                print_centered_header("EDIT DETAILS");
                printf("  Which field do you want to edit?\n");
                printf("--------------------------------------------------------------------------------\n");
                printf("  [1] Product Name\n");
                printf("  [2] Category\n");
                printf("  [3] Sub-Category\n");
                printf("  [4] Condition\n");
                printf("  [5] Price\n");
                printf("  [6] Stock Quantity\n");
                printf("  [7] Contact\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char edit_choice;
                if (scanf(" %c", &edit_choice) != 1)
                {
                    clear_input_buffer();
                    printf("  [ERROR] Invalid input.\n");
                    continue;
                }
                clear_input_buffer();
                edit_choice = toupper((unsigned char)edit_choice);

                if (edit_choice == 'B' || edit_choice == '0')
                    break;

                if (edit_choice == '1')
                {
                    printf("  Enter new Product Name: ");
                    fgets(title, sizeof(title), stdin);
                    title[strcspn(title, "\n")] = 0;
                    printf("  ✔ Product Name updated to %s\n", title);
                    printf("  Press any key to go back to Confirm...\n");
                    getchar();
                    break;
                }
                else if (edit_choice == '2')
                {
                    printf("  [1] Uniform\n  [2] Supply\n  [3] Book\n  [4] Other\n");
                    printf("  Enter choice [0-Back]: ");
                    int cat_ch;
                    if (scanf("%d", &cat_ch) == 1)
                    {
                        clear_input_buffer();
                        if (cat_ch == 1)
                            strcpy(category, "Uniform");
                        else if (cat_ch == 2)
                            strcpy(category, "Supply");
                        else if (cat_ch == 3)
                            strcpy(category, "Book");
                        else if (cat_ch == 4)
                            strcpy(category, "Other");
                        printf("  ✔ Category updated to %s\n", category);
                        printf("  Press any key to go back to Confirm...\n");
                        getchar();
                        break;
                    }
                }
                else if (edit_choice == '3')
                {
                    printf("  Enter new Sub-Category [0-Back]: ");
                    fgets(subcategory, sizeof(subcategory), stdin);
                    subcategory[strcspn(subcategory, "\n")] = 0;
                    if (strcmp(subcategory, "0") == 0)
                    {
                        continue;
                    }
                    if (strlen(subcategory) == 0)
                    {
                        strncpy(subcategory, title, sizeof(subcategory) - 1);
                        subcategory[sizeof(subcategory) - 1] = '\0';
                    }
                    printf("  ✔ Sub-Category updated to %s\n", subcategory);
                    printf("  Press any key to go back to Confirm...\n");
                    getchar();
                    break;
                }
                else if (edit_choice == '4')
                {
                    printf("  [1] Brand New\n  [2] Good Condition\n  [3] Worn\n");
                    printf("  Enter choice [0-Back]: ");
                    int cond_ch;
                    if (scanf("%d", &cond_ch) == 1)
                    {
                        clear_input_buffer();
                        if (cond_ch == 1)
                            strcpy(condition, "Brand New");
                        else if (cond_ch == 2)
                            strcpy(condition, "Good Condition");
                        else if (cond_ch == 3)
                            strcpy(condition, "Worn");
                        printf("  Condition updated to %s\n", condition);
                        printf("  Press any key to go back to Confirm...\n");
                        getchar();
                        break;
                    }
                }
                else if (edit_choice == '5')
                {
                    printf("  Enter new Price (PHP): ");
                    if (scanf("%f", &price) == 1 && price > 0)
                    {
                        clear_input_buffer();
                        printf("  Price updated to PHP %.2f\n", price);
                        printf("  Press any key to go back to Confirm...\n");
                        getchar();
                        break;
                    }
                    else
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid price.\n");
                    }
                }
                else if (edit_choice == '6')
                {
                    printf("  Enter new Stock Quantity: ");
                    if (scanf("%d", &stock) == 1 && stock > 0)
                    {
                        clear_input_buffer();
                        printf("  Stock updated to %d pcs\n", stock);
                        printf("  Press any key to go back to Confirm...\n");
                        getchar();
                        break;
                    }
                    else
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid stock quantity.\n");
                    }
                }
                else if (edit_choice == '7')
                {
                    while (1)
                    {
                        printf("  [1]  Phone Number\n");
                        printf("  [2]  Messenger Name\n");
                        printf("  Enter choice [0-Back]: ");
                        int edit_contact_choice;
                        if (scanf("%d", &edit_contact_choice) != 1)
                        {
                            clear_input_buffer();
                            printf("  [ERROR] Invalid input.\n");
                            continue;
                        }
                        clear_input_buffer();

                        if (edit_contact_choice == 0)
                            break;

                        if (edit_contact_choice == 1)
                        {
                            printf("  Enter Phone Number [0-Back]: ");
                            {
                                char temp[16] = {0};
                                if (!fgets(temp, sizeof(temp), stdin))
                                    return;
                                if (strchr(temp, '\n') == NULL)
                                {
                                    int c;
                                    while ((c = getchar()) != '\n' && c != EOF);
                                }
                                temp[strcspn(temp, "\n")] = 0;
                                strncpy(contact, temp, sizeof(contact) - 1);
                                contact[sizeof(contact) - 1] = '\0';
                            }
                            if (strcmp(contact, "0") == 0)
                                continue;
                            if (!validate_contact(contact))
                            {
                                printf("  [ERROR] Enter exactly 10 digits starting with 9.\n");
                                continue;
                            }
                            contact_type = 1;
                            break;
                        }
                        else if (edit_contact_choice == 2)
                        {
                            printf("  Enter Messenger Name [0-Back]: ");
                            fgets(contact, sizeof(contact), stdin);
                            contact[strcspn(contact, "\n")] = 0;
                            if (strcmp(contact, "0") == 0)
                                continue;
                            if (strlen(contact) == 0)
                            {
                                printf("  [ERROR] Messenger name cannot be empty.\n");
                                continue;
                            }
                            contact_type = 2;
                            break;
                        }
                        else
                        {
                            printf("  [ERROR] Invalid choice.\n");
                            continue;
                        }
                    }
                    printf("  Contact updated to %s\n", contact);
                    printf("  Press any key to go back to Confirm...\n");
                    getchar();
                    break;
                }
                else
                {
                    printf("  [ERROR] Invalid choice.\n");
                }
            }
            continue;
        }

        if (choice == 'P')
        {
    // Post the item (category stored lowercase for catalog / hash / buyer browse)
            char category_db[50];
            strncpy(category_db, category, sizeof(category_db) - 1);
            category_db[sizeof(category_db) - 1] = '\0';
            to_lowercase(category_db);

            int new_id = post_item(catalog, title, category_db, condition, price, seller_name, contact, stock);

            if (new_id != -1)
            {
                ht_insert(ht, category_db, new_id);

    // Get current timestamp
                char timestamp[50];
                get_timestamp(timestamp, sizeof(timestamp));

                print_centered_header("ITEM POSTED!");
                printf("  Logged in as: %s (Seller)\n", user->display_name);
                printf("\n");
                printf("  Your item has been successfully posted!\n");
                printf("--------------------------------------------------------------------------------\n");
                printf("  Item ID       : #%d\n", new_id);
                printf("  Product Name  : %s\n", title);
                printf("  Category      : %s > %s\n", category, subcategory);
                printf("  Condition     : %s\n", condition);
                printf("  Price         : PHP %.2f\n", price);
                printf("  Stock         : %d pcs\n", stock);
                printf("  Seller        : %s\n", seller_name);
                if (contact_type == 1)
                    printf("  Contact       : %s (Phone Number)\n", contact);
                else if (contact_type == 2)
                    printf("  Contact       : %s (Messenger)\n", contact);
                else
                    printf("  Contact       : %s\n", contact);
                printf("  Date Posted   : %s\n", timestamp);
                printf("  Status        : Active\n");
                printf("--------------------------------------------------------------------------------\n");
                printf("  [A] Add Another Product   [V] View My Listings   [B] Back to Seller Menu\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char final_choice;
                if (scanf(" %c", &final_choice) == 1)
                {
                    clear_input_buffer();
                    final_choice = toupper((unsigned char)final_choice);

                    if (final_choice == 'A')
                    {
                        seller_add_item_step_by_step(catalog, q, ht, user);
                    }
                    else if (final_choice == 'V')
                    {
                        seller_view_listings(catalog, q, ht, user);
                        return;
                    }
                    else if (final_choice == 'B')
                    {
                        return;
                    }
                    else if (final_choice == '0')
                    {
                        return;
                    }
                }
            }
            return;
        }

        printf("  [ERROR] Invalid choice.\n");
    }
}

    // --- Helper function to get seller name from item ID ---
const char *get_seller_from_item_id(int item_id)
{
    if (item_id >= 1001 && item_id <= 1010)
    {
        if (item_id == 1001)
            return "Maria Santos";
        else if (item_id == 1002)
            return "Juan Reyes";
        else if (item_id == 1003)
            return "Ana Cruz";
        else if (item_id == 1004)
            return "Carlos Lopez";
        else if (item_id == 1005)
            return "Rosa Garcia";
        else if (item_id == 1006)
            return "Pedro Martinez";
        else if (item_id == 1007)
            return "Sofia Reyes";
        else if (item_id == 1008)
            return "Miguel Torres";
        else if (item_id == 1009)
            return "Elena Santos";
        else if (item_id == 1010)
            return "Diego Cruz";
    }
    else if (item_id >= 2001 && item_id <= 2010)
    {
        if (item_id == 2001)
            return "Lt. Garcia";
        else if (item_id == 2002)
            return "Sgt. Ramos";
        else if (item_id == 2003)
            return "Cadet Lopez";
        else if (item_id == 2004)
            return "Maj. Santos";
        else if (item_id == 2005)
            return "Sgt. Dela Cruz";
        else if (item_id == 2006)
            return "Cpt. Morales";
        else if (item_id == 2007)
            return "Lt. Cruz";
        else if (item_id == 2008)
            return "Cadet Reyes";
        else if (item_id == 2009)
            return "Maj. Garcia";
        else if (item_id == 2010)
            return "Sgt. Torres";
    }
    else if (item_id >= 3001 && item_id <= 3009)
    {
        if (item_id == 3001)
            return "Coach Rivera";
        else if (item_id == 3002)
            return "Juan PE Teacher";
        else if (item_id == 3003)
            return "Maria Sports";
        else if (item_id == 3004)
            return "Luis Fitness";
        else if (item_id == 3005)
            return "Coach Santos";
        else if (item_id == 3006)
            return "Ana Fitness";
        else if (item_id == 3007)
            return "Carlos Gym";
        else if (item_id == 3008)
            return "Rosa Sports";
        else if (item_id == 3009)
            return "Miguel Coach";
    }
    else if (item_id >= 4001 && item_id <= 4010)
    {
        if (item_id == 4001)
            return "Maria Gonzalez";
        else if (item_id == 4002)
            return "Prof. Alvarez";
        else if (item_id == 4003)
            return "Student Council";
        else if (item_id == 4004)
            return "Art Teacher";
        else if (item_id == 4005)
            return "Anna Store";
        else if (item_id == 4006)
            return "Carlos Supplies";
        else if (item_id == 4007)
            return "Science Dept";
        else if (item_id == 4008)
            return "Library";
        else if (item_id == 4009)
            return "Math Teacher";
        else if (item_id == 4010)
            return "Book Store";
    }
    else if (item_id >= 5001 && item_id <= 5009)
    {
        if (item_id == 5001)
            return "Senior Student";
        else if (item_id == 5002)
            return "Faculty Member";
        else if (item_id == 5003)
            return "Book Collector";
        else if (item_id == 5004)
            return "Paulo Books";
        else if (item_id == 5005)
            return "Math Student";
        else if (item_id == 5006)
            return "Science Teacher";
        else if (item_id == 5007)
            return "English Dept";
        else if (item_id == 5008)
            return "History Teacher";
        else if (item_id == 5009)
            return "Computer Lab";
    }
    else if (item_id >= 6001 && item_id <= 6010)
    {
        if (item_id == 6001)
            return "Sports Club";
        else if (item_id == 6002)
            return "PE Department";
        else if (item_id == 6003)
            return "Science Lab";
        else if (item_id == 6004)
            return "AV Technician";
        else if (item_id == 6005)
            return "Security Office";
        else if (item_id == 6006)
            return "Basketball Team";
        else if (item_id == 6007)
            return "Volleyball Club";
        else if (item_id == 6008)
            return "Chemistry Dept";
        else if (item_id == 6009)
            return "IT Department";
        else if (item_id == 6010)
            return "Admin Office";
    }
    return NULL;
}

    // --- Helper function to display inquiries for a specific seller ---
int display_inquiries_for_seller(InquiryQueue *q, const char *seller_name)
{
    if (is_queue_empty(q))
    {
        printf("  No pending inquiries.\n");
        return 0;
    }

    InquiryNode *current = q->front;
    int pos = 1;
    int found = 0;

    printf("  %-4s %-12s %-15s %-12s %s\n",
           "#", "Item ID", "Buyer", "Contact", "Message");
    printf("  %s\n", "-------------------------------------------------------------");

    while (current != NULL)
    {
        const char *item_seller = get_seller_from_item_id(current->item_id);
        if (item_seller && strcasecmp(item_seller, seller_name) == 0)
        {
            printf("  %-4d %-12d %-15s %-12s %s\n",
                   pos,
                   current->item_id,
                   current->buyer_name,
                   current->buyer_contact,
                   current->message);
            found++;
            pos++;
        }
        current = current->next;
    }

    if (found == 0)
    {
        printf("  No pending inquiries for your items.\n");
    }
    else
    {
        printf("\n  Total pending inquiries for you: %d\n", found);
    }

    return found;
}

InquiryNode *get_seller_inquiry_by_index(InquiryQueue *q, const char *seller_name, int index)
{
    if (index < 1 || is_queue_empty(q))
    {
        return NULL;
    }

    InquiryNode *current = q->front;
    int pos = 0;

    while (current != NULL)
    {
        const char *item_seller = get_seller_from_item_id(current->item_id);
        if (item_seller && strcasecmp(item_seller, seller_name) == 0)
        {
            pos++;
            if (pos == index)
            {
                return current;
            }
        }
        current = current->next;
    }

    return NULL;
}

    // --- Helper for building a display item list ---
typedef struct DisplayItem
{
    int id;
    char product[50];
    char condition[30];
    float price;
    char seller[50];
    char contact[20];
    int quantity;
} DisplayItem;

static int sold_item_ids[200];
static int sold_item_count = 0;

static int is_item_sold(int item_id)
{
    for (int i = 0; i < sold_item_count; i++)
    {
        if (sold_item_ids[i] == item_id)
            return 1;
    }
    return 0;
}

static void record_item_sold(int item_id)
{
    if (item_id <= 0 || is_item_sold(item_id))
        return;
    if (sold_item_count < (int)(sizeof(sold_item_ids) / sizeof(sold_item_ids[0])))
    {
        sold_item_ids[sold_item_count++] = item_id;
    }
}

static void add_display_item(DisplayItem items[], int *count,
                             int id,
                             const char *product,
                             const char *condition,
                             float price,
                             const char *seller,
                             const char *contact,
                             int quantity)
{
    if (is_item_sold(id))
        return;
    if (*count >= 56)
        return;
    DisplayItem *item = &items[*count];
    item->id = id;
    strncpy(item->product, product, sizeof(item->product) - 1);
    item->product[sizeof(item->product) - 1] = '\0';
    strncpy(item->condition, condition, sizeof(item->condition) - 1);
    item->condition[sizeof(item->condition) - 1] = '\0';
    item->price = price;
    strncpy(item->seller, seller, sizeof(item->seller) - 1);
    item->seller[sizeof(item->seller) - 1] = '\0';
    strncpy(item->contact, contact, sizeof(item->contact) - 1);
    item->contact[sizeof(item->contact) - 1] = '\0';
    item->quantity = quantity;
    (*count)++;
}

static void sort_display_items_by_price(DisplayItem items[], int count, int ascending)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            int should_swap = ascending
                                  ? (items[j].price > items[j + 1].price)
                                  : (items[j].price < items[j + 1].price);
            if (should_swap)
            {
                DisplayItem temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

static void sort_display_items_by_name(DisplayItem items[], int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        int min_idx = i;
        for (int j = i + 1; j < count; j++)
        {
            if (strcasecmp(items[j].product, items[min_idx].product) < 0)
            {
                min_idx = j;
            }
        }
        if (min_idx != i)
        {
            DisplayItem temp = items[i];
            items[i] = items[min_idx];
            items[min_idx] = temp;
        }
    }
}

static int filter_display_items_by_price(DisplayItem source[], int source_count,
                                         DisplayItem target[], int *target_count,
                                         float min_price, float max_price)
{
    int count = 0;
    for (int i = 0; i < source_count && count < *target_count; i++)
    {
        if (source[i].price >= min_price && source[i].price <= max_price)
        {
            target[count++] = source[i];
        }
    }
    *target_count = count;
    return count;
}

static Item *find_catalog_item_for_display(Catalog *catalog, DisplayItem *item)
{
    if (catalog == NULL || item == NULL)
        return NULL;

    for (Item *it = catalog->head; it != NULL; it = it->next)
    {
        if (strcasecmp(it->title, item->product) == 0 &&
            strcasecmp(it->condition, item->condition) == 0 &&
            strcasecmp(it->seller, item->seller) == 0 &&
            strcasecmp(it->contact, item->contact) == 0 &&
            it->price == item->price)
        {
            return it;
        }
    }
    return NULL;
}

static int buy_now_item(Catalog *catalog, TxStack *stack, DisplayItem *item, User *buyer)
{
    Item *catalog_item = get_item_by_id(catalog, item->id);
    if (catalog_item == NULL)
    {
        catalog_item = find_catalog_item_for_display(catalog, item);
        if (catalog_item != NULL)
        {
            item->id = catalog_item->id;
        }
    }

    if (catalog_item != NULL)
    {
        if (catalog_item->is_sold)
        {
            printf("  [ERROR] Unable to complete purchase. Item may no longer be available.\n");
            return 0;
        }
    }
    else if (is_item_sold(item->id))
    {
        printf("  [ERROR] Unable to complete purchase. Item may no longer be available.\n");
        return 0;
    }

    // Create a temporary cart for Buy Now checkout
    ShoppingCart temp_cart;
    init_cart(&temp_cart);
    add_to_cart(&temp_cart, item->id, item->product, item->condition, item->seller, item->contact, buyer->username, item->price, 1);

    // Display checkout for Buy Now
    int order_placed = checkout_display(&temp_cart, buyer->username, stack, catalog, 1);

    if (order_placed == 1)
    {
    // Catalog stock already updated in checkout; sync sold-id cache when fully sold
        if (catalog_item != NULL)
        {
            if (catalog_item->is_sold)
                record_item_sold(item->id);
        }
        else
        {
            record_item_sold(item->id);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

static void format_category_display(const char *category, const char *subcategory, char *display_buf, int buf_size)
{
    if (strcasecmp(category, "uniform") == 0)
    {
        if (strstr(subcategory, "blouse") || strstr(subcategory, "pants"))
        {
            snprintf(display_buf, buf_size, "Uniform > School Uniform > %s",
                     strcasecmp(subcategory, "blouse") == 0 ? "Blouse" : "Pants");
        }
        else if (strstr(subcategory, "rotc"))
        {
            if (strcasecmp(subcategory, "rotc jacket") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > Jacket");
            }
            else if (strcasecmp(subcategory, "rotc combat shoes") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > Combat Shoes");
            }
            else if (strcasecmp(subcategory, "rotc t-shirt") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > T-Shirt");
            }
            else if (strcasecmp(subcategory, "rotc bullcap") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > Bullcap");
            }
            else if (strcasecmp(subcategory, "rotc belt") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > Belt");
            }
            else if (strcasecmp(subcategory, "rotc uniform") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > ROTC Uniform > Full ROTC Uniform");
            }
        }
        else if (strstr(subcategory, "pe"))
        {
            if (strcasecmp(subcategory, "pe t-shirt") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > PATHFIT Uniform > PE T-Shirt");
            }
            else if (strcasecmp(subcategory, "pe pants") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > PATHFIT Uniform > PE Pants");
            }
            else if (strcasecmp(subcategory, "pathfit uniform") == 0)
            {
                snprintf(display_buf, buf_size, "Uniform > PATHFIT Uniform > Full PATHFIT Uniform");
            }
        }
    }
    else if (strcasecmp(category, "supply") == 0)
    {
        const char *item_name = "";
        if (strcasecmp(subcategory, "school bag") == 0)
            item_name = "School Bag";
        else if (strcasecmp(subcategory, "calculator") == 0)
            item_name = "Calculator";
        else if (strcasecmp(subcategory, "bond paper") == 0)
            item_name = "Bond Paper";
        else if (strcasecmp(subcategory, "ruler") == 0)
            item_name = "Ruler";
        else if (strcasecmp(subcategory, "notebook") == 0)
            item_name = "Notebook";
        else if (strcasecmp(subcategory, "id lace") == 0)
            item_name = "ID Lace";
        else if (strcasecmp(subcategory, "clipboard") == 0)
            item_name = "Clipboard";
        else if (strcasecmp(subcategory, "usb flash drive") == 0)
            item_name = "USB Flash Drive";
        else if (strcasecmp(subcategory, "printer ink") == 0)
            item_name = "Printer Ink";
        else if (strcasecmp(subcategory, "whiteboard marker set") == 0)
            item_name = "Whiteboard Marker Set";
        else if (strcasecmp(subcategory, "extension cord") == 0)
            item_name = "Extension Cord";
        snprintf(display_buf, buf_size, "SUPPLY > %s", item_name);
    }
    else if (strcasecmp(category, "book") == 0)
    {
        const char *item_name = "";
        if (strcasecmp(subcategory, "algebra textbook") == 0)
            item_name = "Algebra Textbook";
        else if (strcasecmp(subcategory, "chemistry textbook") == 0)
            item_name = "Chemistry Textbook";
        else if (strcasecmp(subcategory, "english literature") == 0)
            item_name = "English Literature";
        else if (strcasecmp(subcategory, "biology textbook") == 0)
            item_name = "Biology Textbook";
        else if (strcasecmp(subcategory, "geometry textbook") == 0)
            item_name = "Geometry Textbook";
        else if (strcasecmp(subcategory, "physics textbook") == 0)
            item_name = "Physics Textbook";
        else if (strcasecmp(subcategory, "grammar book") == 0)
            item_name = "Grammar Book";
        else if (strcasecmp(subcategory, "world history") == 0)
            item_name = "World History";
        else if (strcasecmp(subcategory, "programming book") == 0)
            item_name = "Programming Book";
        snprintf(display_buf, buf_size, "BOOK > %s", item_name);
    }
    else if (strcasecmp(category, "other") == 0)
    {
        const char *item_name = "";
        if (strcasecmp(subcategory, "usb flash drive") == 0)
            item_name = "USB Flash Drive";
        else if (strcasecmp(subcategory, "printer ink") == 0)
            item_name = "Printer Ink";
        else if (strcasecmp(subcategory, "extension cord") == 0)
            item_name = "Extension Cord";
        else if (strcasecmp(subcategory, "clipboard") == 0)
            item_name = "Clipboard";
        else if (strcasecmp(subcategory, "other_misc") == 0)
            item_name = "More items";
        else
            item_name = subcategory;
        snprintf(display_buf, buf_size, "OTHER > %s", item_name);
    }
}

static int str_contains_ic(const char *haystack, const char *needle)
{
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    size_t i, j;

    if (nlen == 0)
        return 1;
    if (nlen > hlen)
        return 0;
    for (i = 0; i + nlen <= hlen; i++)
    {
        int match = 1;
        for (j = 0; j < nlen; j++)
        {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j]))
            {
                match = 0;
                break;
            }
        }
        if (match)
            return 1;
    }
    return 0;
}

static int browse_other_demo_title(const char *title)
{
    return strcasecmp(title, "USB Flash Drive") == 0
        || strcasecmp(title, "Printer Ink") == 0
        || strcasecmp(title, "Extension Cord") == 0
        || strcasecmp(title, "Clipboard") == 0;
}

static int catalog_item_matches_other_browse(const Item *it, const char *sub_lc)
{
    if (it->is_sold)
        return 0;
    if (strcasecmp(it->category, "other") != 0)
        return 0;

    if (strcasecmp(sub_lc, "other_misc") == 0)
        return !browse_other_demo_title(it->title);

    if (strcasecmp(sub_lc, "usb flash drive") == 0)
        return strcasecmp(it->title, "USB Flash Drive") == 0
            || str_contains_ic(it->title, "USB")
            || str_contains_ic(it->title, "Flash");

    if (strcasecmp(sub_lc, "printer ink") == 0)
        return strcasecmp(it->title, "Printer Ink") == 0
            || (str_contains_ic(it->title, "Printer") && str_contains_ic(it->title, "Ink"));

    if (strcasecmp(sub_lc, "extension cord") == 0)
        return strcasecmp(it->title, "Extension Cord") == 0
            || str_contains_ic(it->title, "Extension");

    if (strcasecmp(sub_lc, "clipboard") == 0)
        return strcasecmp(it->title, "Clipboard") == 0
            || str_contains_ic(it->title, "Clipboard");

    return 0;
}

static void append_catalog_items_for_other_browse(Catalog *catalog, DisplayItem items[], int *item_count,
                                                  const char *category_lc, const char *sub_lc)
{
    if (strcasecmp(category_lc, "other") != 0)
        return;

    for (Item *it = catalog->head; it != NULL && *item_count < 56; it = it->next)
    {
        if (!catalog_item_matches_other_browse(it, sub_lc))
            continue;

        int dup = 0;
        for (int i = 0; i < *item_count; i++)
        {
            if (items[i].id == it->id)
            {
                dup = 1;
                break;
            }
        }
        if (dup)
            continue;

        add_display_item(items, item_count, it->id, it->title, it->condition,
                         it->price, it->seller, it->contact, it->quantity);
    }
}

    // --- Helper function for product type filtering ---
void display_items_by_subcategory(Catalog *catalog, ShoppingCart *cart, InquiryQueue *q, TxStack *stack, const char *category, const char *subcategory, User *user)
{
    DisplayItem items[56];
    int item_count = 0;

    if (strcasecmp(category, "uniform") == 0)
    {
        if (strcasecmp(subcategory, "blouse") == 0)
        {
            add_display_item(items, &item_count, 1001, "Blouse", "Brand New", 350.00f, "Maria Santos", "09171234567", 5);
            add_display_item(items, &item_count, 1005, "Blouse", "Good Condition", 250.00f, "Rosa Garcia", "09281234567", 3);
            add_display_item(items, &item_count, 1009, "Blouse", "Brand New", 380.00f, "Elena Santos", "09351234567", 1);
        }
        else if (strcasecmp(subcategory, "pants") == 0)
        {
            add_display_item(items, &item_count, 1002, "Pants", "Like New", 280.00f, "Juan Reyes", "09471234567", 4);
            add_display_item(items, &item_count, 1006, "Pants", "Brand New", 320.00f, "Pedro Martinez", "09561234567", 2);
            add_display_item(items, &item_count, 1010, "Pants", "Good Condition", 220.00f, "Diego Cruz", "09671234567", 1);
        }
        else if (strcasecmp(subcategory, "black shoes") == 0)
        {
            add_display_item(items, &item_count, 1003, "Black Shoes", "Good Condition", 200.00f, "Ana Cruz", "09781234567", 1);
            add_display_item(items, &item_count, 1007, "Black Shoes", "Like New", 180.00f, "Sofia Reyes", "09891234567", 1);
        }
        else if (strcasecmp(subcategory, "necktie") == 0)
        {
            add_display_item(items, &item_count, 1004, "Necktie", "Brand New", 120.00f, "Carlos Lopez", "09911234567", 1);
            add_display_item(items, &item_count, 1008, "Necktie", "Good Condition", 90.00f, "Miguel Torres", "09182345678", 1);
        }
        else if (strcasecmp(subcategory, "rotc jacket") == 0)
        {
            add_display_item(items, &item_count, 2001, "Type A Jacket", "Brand New", 450.00f, "Lt. Garcia", "09293456789", 1);
            add_display_item(items, &item_count, 2006, "Type A Jacket", "Like New", 420.00f, "Cpt. Morales", "09314567890", 1);
        }
        else if (strcasecmp(subcategory, "rotc combat shoes") == 0)
        {
            add_display_item(items, &item_count, 2002, "Combat Shoes", "Like New", 380.00f, "Sgt. Ramos", "09425678901", 1);
            add_display_item(items, &item_count, 2007, "Combat Shoes", "Good Condition", 350.00f, "Lt. Cruz", "09536789012", 1);
        }
        else if (strcasecmp(subcategory, "rotc t-shirt") == 0)
        {
            add_display_item(items, &item_count, 2003, "Type C T-Shirt", "Good Condition", 180.00f, "Cadet Lopez", "09647890123", 1);
            add_display_item(items, &item_count, 2008, "Type C T-Shirt", "Brand New", 200.00f, "Cadet Reyes", "09758901234", 1);
        }
        else if (strcasecmp(subcategory, "rotc bullcap") == 0)
        {
            add_display_item(items, &item_count, 2004, "Bullcap", "Brand New", 150.00f, "Maj. Santos", "09869012345", 1);
            add_display_item(items, &item_count, 2009, "Bullcap", "Like New", 130.00f, "Maj. Garcia", "09970123456", 1);
        }
        else if (strcasecmp(subcategory, "rotc belt") == 0)
        {
            add_display_item(items, &item_count, 2005, "Garrison Belt", "Good Condition", 120.00f, "Sgt. Dela Cruz", "09181239876", 1);
            add_display_item(items, &item_count, 2010, "Garrison Belt", "Brand New", 140.00f, "Sgt. Torres", "09282340987", 1);
        }
        else if (strcasecmp(subcategory, "rotc uniform") == 0)
        {
            add_display_item(items, &item_count, 2001, "Type A Jacket", "Brand New", 450.00f, "Lt. Garcia", "09151234101", 1);
            add_display_item(items, &item_count, 2006, "Type A Jacket", "Like New", 420.00f, "Cpt. Morales", "09151234106", 1);
            add_display_item(items, &item_count, 2002, "Combat Shoes", "Like New", 380.00f, "Sgt. Ramos", "09151234102", 1);
            add_display_item(items, &item_count, 2007, "Combat Shoes", "Good Condition", 350.00f, "Lt. Cruz", "09151234107", 1);
            add_display_item(items, &item_count, 2003, "Type C T-Shirt", "Good Condition", 180.00f, "Cadet Lopez", "09151234103", 1);
            add_display_item(items, &item_count, 2008, "Type C T-Shirt", "Brand New", 200.00f, "Cadet Reyes", "09151234108", 1);
            add_display_item(items, &item_count, 2004, "Bullcap", "Brand New", 150.00f, "Maj. Santos", "09151234104", 1);
            add_display_item(items, &item_count, 2009, "Bullcap", "Like New", 130.00f, "Maj. Garcia", "09151234109", 1);
            add_display_item(items, &item_count, 2005, "Garrison Belt", "Good Condition", 120.00f, "Sgt. Dela Cruz", "09151234105", 1);
            add_display_item(items, &item_count, 2010, "Garrison Belt", "Brand New", 140.00f, "Sgt. Torres", "09151234110", 1);
        }
        else if (strcasecmp(subcategory, "pe t-shirt") == 0)
        {
            add_display_item(items, &item_count, 3001, "PE T-Shirt", "Brand New", 220.00f, "Coach Rivera", "09393451098", 1);
            add_display_item(items, &item_count, 3003, "PE T-Shirt", "Good Condition", 180.00f, "Maria Sports", "09404562109", 1);
            add_display_item(items, &item_count, 3005, "PE T-Shirt", "Like New", 240.00f, "Coach Santos", "09515673210", 1);
            add_display_item(items, &item_count, 3007, "PE T-Shirt", "Brand New", 250.00f, "Carlos Gym", "09626784321", 1);
            add_display_item(items, &item_count, 3009, "PE T-Shirt", "Good Condition", 190.00f, "Miguel Coach", "09737895432", 1);
        }
        else if (strcasecmp(subcategory, "pe pants") == 0)
        {
            add_display_item(items, &item_count, 3002, "PE Pants", "Like New", 260.00f, "Juan PE Teacher", "09848906543", 1);
            add_display_item(items, &item_count, 3004, "PE Pants", "Brand New", 280.00f, "Luis Fitness", "09959017654", 1);
            add_display_item(items, &item_count, 3006, "PE Pants", "Good Condition", 230.00f, "Ana Fitness", "09160128765", 1);
            add_display_item(items, &item_count, 3008, "PE Pants", "Like New", 270.00f, "Rosa Sports", "09271239876", 1);
        }
        else if (strcasecmp(subcategory, "pathfit uniform") == 0)
        {
            add_display_item(items, &item_count, 3001, "PE T-Shirt", "Brand New", 220.00f, "Coach Rivera", "09393451098", 1);
            add_display_item(items, &item_count, 3003, "PE T-Shirt", "Good Condition", 180.00f, "Maria Sports", "09404562109", 1);
            add_display_item(items, &item_count, 3005, "PE T-Shirt", "Like New", 240.00f, "Coach Santos", "09515673210", 1);
            add_display_item(items, &item_count, 3007, "PE T-Shirt", "Brand New", 250.00f, "Carlos Gym", "09626784321", 1);
            add_display_item(items, &item_count, 3009, "PE T-Shirt", "Good Condition", 190.00f, "Miguel Coach", "09737895432", 1);
            add_display_item(items, &item_count, 3002, "PE Pants", "Like New", 260.00f, "Juan PE Teacher", "09848906543", 1);
            add_display_item(items, &item_count, 3004, "PE Pants", "Brand New", 280.00f, "Luis Fitness", "09959017654", 1);
            add_display_item(items, &item_count, 3006, "PE Pants", "Good Condition", 230.00f, "Ana Fitness", "09160128765", 1);
            add_display_item(items, &item_count, 3008, "PE Pants", "Like New", 270.00f, "Rosa Sports", "09271239876", 1);
        }
    }
    else if (strcasecmp(category, "supply") == 0)
    {
        if (strcasecmp(subcategory, "school bag") == 0)
        {
            add_display_item(items, &item_count, 4001, "School Bag", "Brand New", 450.00f, "Maria Gonzalez", "09382340987", 1);
            add_display_item(items, &item_count, 4006, "School Bag", "Like New", 400.00f, "Carlos Supplies", "09493451098", 1);
        }
        else if (strcasecmp(subcategory, "calculator") == 0)
        {
            add_display_item(items, &item_count, 4002, "Calculator", "Like New", 380.00f, "Prof. Alvarez", "09504562109", 1);
            add_display_item(items, &item_count, 4007, "Calculator", "Good Condition", 320.00f, "Science Dept", "09171234567", 1);
        }
        else if (strcasecmp(subcategory, "bond paper") == 0)
        {
            add_display_item(items, &item_count, 4003, "Bond Paper", "Brand New", 80.00f, "Student Council", "09281234567", 1);
            add_display_item(items, &item_count, 4008, "Bond Paper", "Brand New", 90.00f, "Library", "09351234567", 1);
        }
        else if (strcasecmp(subcategory, "ruler") == 0)
        {
            add_display_item(items, &item_count, 4004, "Ruler (30cm)", "Brand New", 30.00f, "Art Teacher", "09471234567", 1);
            add_display_item(items, &item_count, 4009, "Ruler (30cm)", "Like New", 25.00f, "Math Teacher", "09561234567", 1);
        }
        else if (strcasecmp(subcategory, "notebook") == 0)
        {
            add_display_item(items, &item_count, 4005, "Notebook", "Good Condition", 50.00f, "Anna Store", "09671234567", 1);
            add_display_item(items, &item_count, 4010, "Notebook", "Brand New", 60.00f, "Book Store", "09781234567", 1);
        }
        else if (strcasecmp(subcategory, "id lace") == 0)
        {
            add_display_item(items, &item_count, 4011, "ID Lace", "Brand New", 40.00f, "Student Council", "09182340987", 1);
            add_display_item(items, &item_count, 4012, "ID Lace", "Like New", 25.00f, "Campus Store", "09293451098", 1);
        }
        else if (strcasecmp(subcategory, "clipboard") == 0)
        {
            add_display_item(items, &item_count, 4013, "Clipboard", "Brand New", 75.00f, "Guidance Office", "09737895432", 1);
            add_display_item(items, &item_count, 4014, "Clipboard", "Like New", 55.00f, "Class Adviser", "09848906543", 1);
        }
        else if (strcasecmp(subcategory, "usb flash drive") == 0)
        {
            add_display_item(items, &item_count, 4015, "USB Flash Drive", "Like New", 250.00f, "IT Office", "09869012345", 1);
            add_display_item(items, &item_count, 4016, "USB Flash Drive", "Brand New", 320.00f, "Computer Lab", "09970123456", 1);
        }
        else if (strcasecmp(subcategory, "printer ink") == 0)
        {
            add_display_item(items, &item_count, 4017, "Printer Ink", "Brand New", 450.00f, "Admin Office", "09181239876", 1);
            add_display_item(items, &item_count, 4018, "Printer Ink", "Brand New", 500.00f, "Registrar Staff", "09282340987", 1);
        }
        else if (strcasecmp(subcategory, "whiteboard marker set") == 0)
        {
            add_display_item(items, &item_count, 4019, "Whiteboard Marker Set", "Brand New", 180.00f, "Math Teacher", "09393451098", 1);
            add_display_item(items, &item_count, 4020, "Whiteboard Marker Set", "Like New", 140.00f, "Science Teacher", "09404562109", 1);
        }
        else if (strcasecmp(subcategory, "extension cord") == 0)
        {
            add_display_item(items, &item_count, 4021, "Extension Cord", "Good Condition", 220.00f, "Facilities Staff", "09515673210", 1);
            add_display_item(items, &item_count, 4022, "Extension Cord", "Brand New", 280.00f, "Electrical Club", "09626784321", 1);
        }
    }
    else if (strcasecmp(category, "book") == 0)
    {
        if (strcasecmp(subcategory, "algebra textbook") == 0)
        {
            add_display_item(items, &item_count, 5001, "Algebra Textbook", "Like New", 300.00f, "Senior Student", "09891234567", 1);
        }
        else if (strcasecmp(subcategory, "chemistry textbook") == 0)
        {
            add_display_item(items, &item_count, 5002, "Chemistry Advanced", "Good Condition", 350.00f, "Faculty Member", "09911234567", 1);
        }
        else if (strcasecmp(subcategory, "english literature") == 0)
        {
            add_display_item(items, &item_count, 5003, "English Literature", "Like New", 280.00f, "Book Collector", "09182345678", 1);
        }
        else if (strcasecmp(subcategory, "biology textbook") == 0)
        {
            add_display_item(items, &item_count, 5004, "Biology Textbook", "Brand New", 400.00f, "Paulo Books", "09293456789", 1);
        }
        else if (strcasecmp(subcategory, "geometry textbook") == 0)
        {
            add_display_item(items, &item_count, 5005, "Geometry Textbook", "Good Condition", 320.00f, "Math Student", "09314567890", 1);
        }
        else if (strcasecmp(subcategory, "physics textbook") == 0)
        {
            add_display_item(items, &item_count, 5006, "Physics Textbook", "Like New", 380.00f, "Science Teacher", "09425678901", 1);
        }
        else if (strcasecmp(subcategory, "grammar book") == 0)
        {
            add_display_item(items, &item_count, 5007, "Grammar Book", "Brand New", 250.00f, "English Dept", "09536789012", 1);
        }
        else if (strcasecmp(subcategory, "world history") == 0)
        {
            add_display_item(items, &item_count, 5008, "World History", "Good Condition", 290.00f, "History Teacher", "09647890123", 1);
        }
        else if (strcasecmp(subcategory, "programming book") == 0)
        {
            add_display_item(items, &item_count, 5009, "Programming Book", "Like New", 350.00f, "Computer Lab", "09758901234", 1);
        }
    }
    else if (strcasecmp(category, "other") == 0)
    {
        if (strcasecmp(subcategory, "usb flash drive") == 0)
        {
            add_display_item(items, &item_count, 6001, "USB Flash Drive", "Like New", 250.00f, "IT Office", "09869012345", 1);
            add_display_item(items, &item_count, 6006, "USB Flash Drive", "Brand New", 320.00f, "Computer Lab", "09970123456", 1);
        }
        else if (strcasecmp(subcategory, "printer ink") == 0)
        {
            add_display_item(items, &item_count, 6002, "Printer Ink", "Brand New", 450.00f, "Admin Office", "09181239876", 1);
            add_display_item(items, &item_count, 6007, "Printer Ink", "Brand New", 500.00f, "Registrar Staff", "09282340987", 1);
        }
        else if (strcasecmp(subcategory, "extension cord") == 0)
        {
            add_display_item(items, &item_count, 6004, "Extension Cord", "Good Condition", 220.00f, "Facilities Staff", "09515673210", 1);
            add_display_item(items, &item_count, 6009, "Extension Cord", "Brand New", 280.00f, "Electrical Club", "09626784321", 1);
        }
        else if (strcasecmp(subcategory, "clipboard") == 0)
        {
            add_display_item(items, &item_count, 6005, "Clipboard", "Brand New", 75.00f, "Guidance Office", "09737895432", 1);
            add_display_item(items, &item_count, 6010, "Clipboard", "Like New", 55.00f, "Class Adviser", "09848906543", 1);
        }
    // other_misc: demo rows only come from live catalog via append below
    }

    append_catalog_items_for_other_browse(catalog, items, &item_count, category, subcategory);

    {
        char category_title[128];
        snprintf(category_title, sizeof(category_title), "CATEGORY: %s > %s", category, subcategory);
        print_centered_header(category_title);
    }
    printf("  No.  ID      Product         Condition        Price        Seller            Contact\n");
    printf("  ------------------------------------------------------------------------------\n");

    for (int i = 0; i < item_count; i++)
    {
        printf("  [%2d]  %-6d  %-15s %-15s PHP %-8.2f %-15s %s\n",
               i + 1,
               items[i].id,
               items[i].product,
               items[i].condition,
               items[i].price,
               items[i].seller,
               items[i].contact);
    }

    printf("  ------------------------------------------------------------------------------\n");
    printf("  %d result(s) found for \"%s\"\n", item_count, subcategory);
    printf("================================================================================\n");
    printf("                     [S] Sort   [F] Filter                                  \n");
    printf("================================================================================\n");

    char action[16];
    while (1)
    {
        printf("  Enter No. to View Product: ");
        if (!fgets(action, sizeof(action), stdin))
        {
            return;
        }
        action[strcspn(action, "\n")] = '\0';

        if (strcasecmp(action, "B") == 0 || strcmp(action, "0") == 0)
        {
            return;
        }
        if (strcasecmp(action, "S") == 0 || strcasecmp(action, "F") == 0)
        {
            printf("  [INFO] Sort and filter are not available yet.\n");
            continue;
        }

        int selected_index = atoi(action);
        if (selected_index < 1 || selected_index > item_count)
        {
            printf("  [ERROR] Please enter a valid number between 1 and %d, or 0 to go back.\n", item_count);
            continue;
        }

        DisplayItem *selected = &items[selected_index - 1];
        while (1)
        {
            char category_display[100];
            format_category_display(category, subcategory, category_display, sizeof(category_display));
            print_centered_header("PRODUCT DETAILS");
            printf("  Name     : %s\n", selected->product);
            printf("  Price    : PHP %.2f\n", selected->price);
            printf("  Seller   : %s\n", selected->seller);
            printf("  Category : %s\n", category_display);
            printf("  Stock    : %d available\n", selected->quantity);
            printf("================================================================================\n");
            printf("  [1] Add to Cart\n");
            printf("  [2] Buy Now\n");
            printf("================================================================================\n");
            printf("  Enter choice [0-Back]: ");

            char action[16];
            if (!fgets(action, sizeof(action), stdin))
            {
                return;
            }
            action[strcspn(action, "\n")] = '\0';

            if (strcasecmp(action, "B") == 0 || strcmp(action, "0") == 0)
            {
                break;
            }

            if (strcmp(action, "1") == 0)
            {
    // Check if item quantity is 1, if so, don't ask, just add 1 to cart
                int quantity;
                if (selected->quantity == 1)
                {
                    quantity = 1;
                    printf("  [INFO] Only 1 item available, adding to cart...\n");
                }
                else
                {
                    printf("  [INFO] %d items available\n", selected->quantity);
                    quantity = prompt_quantity();
                }
                if (add_to_cart(cart, selected->id, selected->product, selected->condition, selected->seller, selected->contact, user->username, selected->price, quantity))
                {
                    display_add_to_cart_success(selected->product, selected->condition, selected->price, selected->seller, quantity, cart);

                    int post_choice = -1;
                    while (1)
                    {
                        if (scanf("%d", &post_choice) != 1)
                        {
                            clear_input_buffer();
                            printf("  [ERROR] Invalid choice.\n");
                            continue;
                        }
                        clear_input_buffer();

                        if (post_choice == 1)
                        {
    // Continue Browsing
                            break;
                        }
                        else if (post_choice == 2)
                        {
                            handle_cart_menu(cart, user->username, stack, catalog);
                            continue;
                        }
                        else if (post_choice == 3)
                        {
    // Checkout Now
                            checkout_display(cart, user->username, stack, catalog, 1);
                            break;
                        }
                        else if (post_choice == 0)
                        {
    // Back to Menu
                            break;
                        }

                        printf("  [ERROR] Please choose 0, 1, 2, or 3.\n");
                    }

                    char message[200] = "Interested in purchasing this item.";
                    if (enqueue_inquiry(q, selected->id, user->username, user->username, message))
                    {
                        printf("  Inquiry sent to seller automatically.\n");
                    }
                    else
                    {
                        printf("  [ERROR] Failed to send inquiry.\n");
                    }
                }
                else
                {
                    printf("  [ERROR] Failed to add item to cart.\n");
                }
                break;
            }

            if (strcmp(action, "2") == 0)
            {
                if (buy_now_item(catalog, stack, selected, user))
                {
                    break;
                }
                continue;
            }

            printf("  [ERROR] Invalid choice. Enter 1, 2, or 0.\n");
        }
        return;
    }
}

void browse_menu(Catalog *catalog);

/* ============================================================
   BUYER MENU
   ============================================================ */
void buyer_menu(Catalog *catalog, InquiryQueue *q, ShoppingCart *cart, TxStack *stack, HashTable *ht, UserList *users, UserHashTable *user_ht, User *user)
{
    int choice;
    int browse_direct = 0;

    do
    {
        if (!browse_direct)
        {
            print_header("BUYER MENU");
            printf("  Logged in as: %s (Buyer)\n", user->display_name);
            printf("  [1] Browse Products\n");
            printf("  [2] Search Product\n");
            printf("  [3] View Cart\n");
            printf("  [4] My Orders / Transaction History\n");
            printf("  [5] View Category Index\n");
            printf("  [6] Delete Account\n");
            printf("  [7] Logout\n");
            printf("\n  Enter choice [0-Back]: ");

            if (scanf("%d", &choice) != 1)
            {
                clear_input_buffer();
                printf("  [ERROR] Invalid input.\n");
                continue;
            }
            clear_input_buffer();
        }
        else
        {
            choice = 1;
            browse_direct = 0;
        }

        if (choice == 1)
        {
    // --- Browse Products ---
            print_header("BROWSE PRODUCTS");
            printf("  [1] Uniform\n");
            printf("  [2] Supply\n");
            printf("  [3] Book\n");
            printf("  [4] Other\n");
            printf("\n  Enter choice [0-Back]: ");

            int cat_choice;
            if (scanf("%d", &cat_choice) != 1)
            {
                clear_input_buffer();
                printf("  [ERROR] Invalid input.\n");
                continue;
            }
            clear_input_buffer();

            char category[50];
            char subcategory[100];
            switch (cat_choice)
            {
            case 0:
                continue;
            case 1:
            {
                strcpy(category, "uniform");
                int browse_uniform = 1;
                while (browse_uniform)
                {
                uniform_menu:
                    print_header("UNIFORM SUBCATEGORIES");
                    printf("  [1] SCHOOL UNIFORM\n");
                    printf("  [2] ROTC UNIFORM\n");
                    printf("  [3] PATHFIT UNIFORM\n");
                    printf("\n  Enter choice [0-Back]: ");

                    int uniform_choice;
                    if (scanf("%d", &uniform_choice) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid input.\n");
                        continue;
                    }
                    clear_input_buffer();

                    if (uniform_choice == 0)
                    {
                        browse_uniform = 0;
                        continue;
                    }

                    switch (uniform_choice)
                    {
                    case 1:
                    {
                        int school_choice;
                    school_uniform_menu:
                        while (1)
                        {
                            print_header("SCHOOL UNIFORM");
                            printf("  [1] Blouse\n");
                            printf("  [2] Pants\n");
                            printf("\n  Enter choice [0-Back]: ");

                            if (scanf("%d", &school_choice) != 1)
                            {
                                clear_input_buffer();
                                printf("  [ERROR] Invalid input.\n");
                                continue;
                            }
                            clear_input_buffer();

                            switch (school_choice)
                            {
                            case 1:
                                strcpy(subcategory, "Blouse");
                                break;
                            case 2:
                                strcpy(subcategory, "Pants");
                                break;
                            case 0:
                                goto uniform_menu;
                            default:
                                printf("  [ERROR] Invalid choice.\n");
                                continue;
                            }
                            break;
                        }
                        display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                        goto school_uniform_menu;
                    }
                    case 2:
                    {
                        int rotc_choice;
                    rotc_uniform_menu:
                        while (1)
                        {
                            print_header("ROTC UNIFORM");
                            printf("  [1] Jacket\n");
                            printf("  [2] Combat Shoes\n");
                            printf("  [3] T-Shirt\n");
                            printf("  [4] Bullcap\n");
                            printf("  [5] Belt\n");
                            printf("  [6] Full ROTC Uniform\n");
                            printf("\n  Enter choice [0-Back]: ");

                            if (scanf("%d", &rotc_choice) != 1)
                            {
                                clear_input_buffer();
                                printf("  [ERROR] Invalid input.\n");
                                continue;
                            }
                            clear_input_buffer();

                            switch (rotc_choice)
                            {
                            case 1:
                                strcpy(subcategory, "ROTC jacket");
                                break;
                            case 2:
                                strcpy(subcategory, "ROTC Combat shoes");
                                break;
                            case 3:
                                strcpy(subcategory, "ROTC T-shirt");
                                break;
                            case 4:
                                strcpy(subcategory, "ROTC Bullcap");
                                break;
                            case 5:
                                strcpy(subcategory, "ROTC Belt");
                                break;
                            case 6:
                                strcpy(subcategory, "ROTC Uniform");
                                break;
                            case 0:
                                goto uniform_menu;
                            default:
                                printf("  [ERROR] Invalid choice.\n");
                                continue;
                            }
                            break;
                        }
                        display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                        goto rotc_uniform_menu;
                    }
                    case 3:
                    {
                        int pathfit_choice;
                    pathfit_uniform_menu:
                        while (1)
                        {
                            print_header("PATHFIT UNIFORM");
                            printf("  [1] PE T-Shirt\n");
                            printf("  [2] PE Pants\n");
                            printf("  [3] Full PATHFIT Uniform\n");
                            printf("\n  Enter choice [0-Back]: ");

                            if (scanf("%d", &pathfit_choice) != 1)
                            {
                                clear_input_buffer();
                                printf("  [ERROR] Invalid input.\n");
                                continue;
                            }
                            clear_input_buffer();

                            switch (pathfit_choice)
                            {
                            case 1:
                                strcpy(subcategory, "PE T-shirt");
                                break;
                            case 2:
                                strcpy(subcategory, "PE Pants");
                                break;
                            case 3:
                                strcpy(subcategory, "Pathfit Uniform");
                                break;
                            case 0:
                                goto uniform_menu;
                            default:
                                printf("  [ERROR] Invalid choice.\n");
                                continue;
                            }
                            break;
                        }
                        display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                        goto pathfit_uniform_menu;
                    }
                    default:
                        printf("  [ERROR] Invalid choice.\n");
                    }
                }
                break;
            }

            case 2:
            {
                strcpy(category, "supply");
                int browse_supply = 1;
                while (browse_supply)
                {
                supply_menu:
                    print_header("SUPPLY SUBCATEGORIES");
                    printf("  [1] School Bag\n");
                    printf("  [2] Calculator\n");
                    printf("  [3] Bond Paper\n");
                    printf("  [4] Ruler\n");
                    printf("  [5] Notebook\n");
                    printf("  [6] ID Lace\n");
                    printf("  [7] Clipboard\n");
                    printf("  [8] USB Flash Drive\n");
                    printf("  [9] Printer Ink\n");
                    printf("  [10] Whiteboard Marker Set\n");
                    printf("  [11] Extension Cord\n");
                    printf("\n  Enter choice [0-Back]: ");

                    int supply_choice;
                    if (scanf("%d", &supply_choice) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid input.\n");
                        continue;
                    }
                    clear_input_buffer();

                    if (supply_choice == 0)
                    {
                        browse_supply = 0;
                        continue;
                    }

                    switch (supply_choice)
                    {
                    case 1:
                        strcpy(subcategory, "school bag");
                        break;
                    case 2:
                        strcpy(subcategory, "calculator");
                        break;
                    case 3:
                        strcpy(subcategory, "bond paper");
                        break;
                    case 4:
                        strcpy(subcategory, "ruler");
                        break;
                    case 5:
                        strcpy(subcategory, "notebook");
                        break;
                    case 6:
                        strcpy(subcategory, "id lace");
                        break;
                    case 7:
                        strcpy(subcategory, "clipboard");
                        break;
                    case 8:
                        strcpy(subcategory, "usb flash drive");
                        break;
                    case 9:
                        strcpy(subcategory, "printer ink");
                        break;
                    case 10:
                        strcpy(subcategory, "whiteboard marker set");
                        break;
                    case 11:
                        strcpy(subcategory, "extension cord");
                        break;
                    default:
                        printf("  [ERROR] Invalid choice.\n");
                        goto supply_menu;
                    }
                    display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                    goto supply_menu;
                }
                break;
            }

            case 3:
            {
                strcpy(category, "book");
                int browse_book = 1;
                while (browse_book)
                {
                book_menu:
                    print_header("BOOK SUBCATEGORIES");
                    printf("  [1] Algebra Textbook\n");
                    printf("  [2] Chemistry Textbook\n");
                    printf("  [3] English Literature\n");
                    printf("  [4] Biology Textbook\n");
                    printf("  [5] Geometry Textbook\n");
                    printf("  [6] Physics Textbook\n");
                    printf("  [7] Grammar Book\n");
                    printf("  [8] World History\n");
                    printf("  [9] Programming Book\n");
                    printf("\n  Enter choice [0-Back]: ");

                    int book_choice;
                    if (scanf("%d", &book_choice) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid input.\n");
                        continue;
                    }
                    clear_input_buffer();

                    if (book_choice == 0)
                    {
                        browse_book = 0;
                        continue;
                    }

                    switch (book_choice)
                    {
                    case 1:
                        strcpy(subcategory, "algebra textbook");
                        break;
                    case 2:
                        strcpy(subcategory, "chemistry textbook");
                        break;
                    case 3:
                        strcpy(subcategory, "english literature");
                        break;
                    case 4:
                        strcpy(subcategory, "biology textbook");
                        break;
                    case 5:
                        strcpy(subcategory, "geometry textbook");
                        break;
                    case 6:
                        strcpy(subcategory, "physics textbook");
                        break;
                    case 7:
                        strcpy(subcategory, "grammar book");
                        break;
                    case 8:
                        strcpy(subcategory, "world history");
                        break;
                    case 9:
                        strcpy(subcategory, "programming book");
                        break;
                    default:
                        printf("  [ERROR] Invalid choice.\n");
                        goto book_menu;
                    }
                    display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                    goto book_menu;
                }
                break;
            }

            case 4:
            {
                strcpy(category, "other");
                int browse_other = 1;
                while (browse_other)
                {
                other_menu:
                    print_header("OTHER SUBCATEGORIES");
                    printf("  [1] USB Flash Drive\n");
                    printf("  [2] Printer Ink\n");
                    printf("  [3] Extension Cord\n");
                    printf("  [4] Clipboard\n");
                    printf("  [5] More items (other listings)\n");
                    printf("\n  Enter choice [0-Back]: ");

                    int other_choice;
                    if (scanf("%d", &other_choice) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid input.\n");
                        continue;
                    }
                    clear_input_buffer();

                    if (other_choice == 0)
                    {
                        browse_other = 0;
                        continue;
                    }

                    switch (other_choice)
                    {
                    case 1:
                        strcpy(subcategory, "usb flash drive");
                        break;
                    case 2:
                        strcpy(subcategory, "printer ink");
                        break;
                    case 3:
                        strcpy(subcategory, "extension cord");
                        break;
                    case 4:
                        strcpy(subcategory, "clipboard");
                        break;
                    case 5:
                        strcpy(subcategory, "other_misc");
                        break;
                    default:
                        printf("  [ERROR] Invalid choice.\n");
                        goto other_menu;
                    }
                    display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                    goto other_menu;
                }
                break;
            }

            default:
                printf("  [ERROR] Invalid choice.\n");
                continue;
            }
        }
        else if (choice == 2)
        {
    // --- Search Product ---
            while (1)
            {
                print_centered_header("SEARCH PRODUCT");
                printf("  Enter product name [0-Back]:");
                char keyword[100];
                if (!fgets(keyword, sizeof(keyword), stdin))
                {
                    break;
                }
                keyword[strcspn(keyword, "\n")] = 0;

                if (strcmp(keyword, "0") == 0)
                {
                    break;
                }

                // Search and collect results
                DisplayItem search_results[50];
                int result_count = 0;

                Item *current = catalog->head;
                char kw_lower[100];
                strncpy(kw_lower, keyword, sizeof(kw_lower) - 1);
                to_lowercase(kw_lower);

                while (current != NULL && result_count < 50)
                {
                    char title_lower[100];
                    char category_lower[100];
                    strncpy(title_lower, current->title, sizeof(title_lower) - 1);
                    strncpy(category_lower, current->category, sizeof(category_lower) - 1);
                    to_lowercase(title_lower);
                    to_lowercase(category_lower);

                    if (!current->is_sold &&
                        (strstr(title_lower, kw_lower) || strstr(category_lower, kw_lower)))
                    {
                        add_display_item(search_results, &result_count, current->id, current->title, current->condition, current->price, current->seller, current->contact, current->quantity);
                    }
                    current = current->next;
                }

                if (result_count == 0)
                {
                    char search_title[128];
                    snprintf(search_title, sizeof(search_title), "SEARCH RESULTS FOR: \"%s\"", keyword);
                    print_centered_header(search_title);
                    printf("   No results found for \"%s\".\n", keyword);
                    printf("================================================================================\n");
                    printf("  Enter choice [0-Back]: ");
                    char back_input[32];
                    if (!fgets(back_input, sizeof(back_input), stdin))
                        break;
                    trim_input(back_input);
                    if (strcmp(back_input, "0") == 0)
                        break;
                    continue;
                }

                {
                    char search_title[128];
                    snprintf(search_title, sizeof(search_title), "SEARCH RESULTS FOR: \"%s\"", keyword);
                    print_centered_header(search_title);
                }
                printf("  No.  Product               ID      Condition        Price        Seller            Contact\n");
                printf("  ----------------------------------------------------------------------------------------------\n");

                for (int i = 0; i < result_count; i++)
                {
                    printf("  [%2d]  %-20s  %-6d  %-15s  PHP %-8.2f %-15s %s\n",
                           i + 1,
                           search_results[i].product,
                           search_results[i].id,
                           search_results[i].condition,
                           search_results[i].price,
                           search_results[i].seller,
                           search_results[i].contact);
                }

                printf("  ----------------------------------------------------------------------------------------------\n");
                printf("  %d result(s) found for \"%s\"\n", result_count, keyword);
                printf("================================================================================\n");
                printf("                        [S] Sort   [F] Filter                                \n");
                printf("================================================================================\n");
                printf("  Enter No. to View Product: ");

                char action[16];
                if (!fgets(action, sizeof(action), stdin))
                {
                    break;
                }
                action[strcspn(action, "\n")] = '\0';

                if (strcasecmp(action, "B") == 0 || strcmp(action, "0") == 0)
                {
                    continue;
                }
                else if (strcasecmp(action, "S") == 0)
                {
                    printf("  [1] Price low to high\n");
                    printf("  [2] Price high to low\n");
                    printf("  [3] Name A-Z\n");
                    printf("  Enter sort option: ");

                    char sort_choice[16];
                    if (!fgets(sort_choice, sizeof(sort_choice), stdin))
                    {
                        continue;
                    }
                    sort_choice[strcspn(sort_choice, "\n")] = '\0';

                    if (strcmp(sort_choice, "1") == 0)
                    {
                        sort_display_items_by_price(search_results, result_count, 1);
                        printf("  [INFO] Results sorted by price (low to high).\n");
                    }
                    else if (strcmp(sort_choice, "2") == 0)
                    {
                        sort_display_items_by_price(search_results, result_count, 0);
                        printf("  [INFO] Results sorted by price (high to low).\n");
                    }
                    else if (strcmp(sort_choice, "3") == 0)
                    {
                        sort_display_items_by_name(search_results, result_count);
                        printf("  [INFO] Results sorted by product name.\n");
                    }
                    else
                    {
                        printf("  [ERROR] Invalid sort option.\n");
                    }
                    continue;
                }
                else if (strcasecmp(action, "F") == 0)
                {
                    printf("  Enter minimum price (PHP): ");
                    float min_price;
                    if (scanf("%f", &min_price) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid minimum price.\n");
                        continue;
                    }
                    printf("  Enter maximum price (PHP): ");
                    float max_price;
                    if (scanf("%f", &max_price) != 1)
                    {
                        clear_input_buffer();
                        printf("  [ERROR] Invalid maximum price.\n");
                        continue;
                    }
                    clear_input_buffer();

                    DisplayItem filtered_results[50];
                    int temp_count = 50;
                    filter_display_items_by_price(search_results, result_count, filtered_results, &temp_count, min_price, max_price);
                    if (temp_count == 0)
                    {
                        printf("  [INFO] No items matched the price range PHP %.2f - PHP %.2f.\n", min_price, max_price);
                        continue;
                    }
                    memcpy(search_results, filtered_results, temp_count * sizeof(DisplayItem));
                    result_count = temp_count;
                    printf("  [INFO] Filter applied. %d item(s) remain.\n", result_count);
                    continue;
                }

                int selected_index = 0;
                selected_index = atoi(action);

                if (selected_index < 1 || selected_index > result_count)
                {
                    printf("  [ERROR] Please enter a valid number between 1 and %d, or 0 to go back.\n", result_count);
                    continue;
                }

                DisplayItem *selected = &search_results[selected_index - 1];

                while (1)
                {
                    print_centered_header("PRODUCT DETAILS");
                    printf("  Name     : %s\n", selected->product);
                    printf("  Price    : PHP %.2f\n", selected->price);
                    printf("  Seller   : %s\n", selected->seller);
                    printf("  Category : Search Result\n");
                    printf("  Stock    : %d available\n", selected->quantity);
                    printf("================================================================================\n");
                    printf("  [1] Add to Cart\n");
                    printf("  [2] Buy Now\n");
                    printf("================================================================================\n");
                    printf("  Enter choice [0-Back]: ");

                    char action[16];
                    if (!fgets(action, sizeof(action), stdin))
                    {
                        break;
                    }
                    action[strcspn(action, "\n")] = '\0';

                    if (strcasecmp(action, "B") == 0 || strcmp(action, "0") == 0)
                    {
                        break;
                    }

                    if (strcmp(action, "1") == 0)
                    {
    // Check if item quantity is 1, if so, don't ask, just add 1 to cart
                        int quantity;
                        if (selected->quantity == 1)
                        {
                            quantity = 1;
                            printf("  [INFO] Only 1 item available, adding to cart...\n");
                        }
                        else
                        {
                            printf("  [INFO] %d items available\n", selected->quantity);
                            quantity = prompt_quantity();
                        }
                        if (add_to_cart(cart, selected->id, selected->product, selected->condition, selected->seller, selected->contact, user->username, selected->price, quantity))
                        {
                            display_add_to_cart_success(selected->product, selected->condition, selected->price, selected->seller, quantity, cart);

                            int post_choice = -1;
                            while (1)
                            {
                                if (scanf("%d", &post_choice) != 1)
                                {
                                    clear_input_buffer();
                                    printf("  [ERROR] Invalid choice.\n");
                                    continue;
                                }
                                clear_input_buffer();

                                if (post_choice == 1)
                                {
    // Continue Browsing
                                    break;
                                }
                                else if (post_choice == 2)
                                {
                                    handle_cart_menu(cart, user->username, stack, catalog);
                                    continue;
                                }
                                else if (post_choice == 3)
                                {
    // Checkout Now
                                    checkout_display(cart, user->username, stack, catalog, 0);
                                    break;
                                }
                                else if (post_choice == 0)
                                {
    // Back to Menu
                                    break;
                                }

                                printf("  [ERROR] Please choose 0, 1, 2, or 3.\n");
                            }

                            char message[200] = "Interested in purchasing this item.";
                            if (enqueue_inquiry(q, selected->id, user->username, user->username, message))
                            {
                                printf("  Inquiry sent to seller automatically.\n");
                            }
                            else
                            {
                                printf("  [ERROR] Failed to send inquiry.\n");
                            }
                        }
                        else
                        {
                            printf("  [ERROR] Failed to add item to cart.\n");
                        }
                        break;
                    }

                    if (strcmp(action, "2") == 0)
                    {
                        if (buy_now_item(catalog, stack, selected, user))
                        {
                            break;
                        }
                        continue;
                    }

                    printf("  [ERROR] Invalid choice. Enter 1, 2, or 0.\n");
                }
                break;
            }
        }
        else if (choice == 3)
        {
    // --- View Cart ---
            int cart_result = handle_cart_menu(cart, user->username, stack, catalog);
            if (cart_result == 1)
            {
                browse_direct = 1;
                continue;
            }
        }
        else if (choice == 4)
        {
    // --- My Orders / Transaction History ---
            while (1)
            {
                print_centered_header("MY ORDERS");
                printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                display_user_orders(stack, user->username);
                printf("\n");
                printf("  [V] View Details   [R] Mark as Received   [C] Cancel Order   [B] Back to Buyer Menu\n");
                printf("\n");
                printf("================================================================================\n");
                printf("  Enter choice [0-Back]: ");

                char order_nav[32];
                if (!fgets(order_nav, sizeof(order_nav), stdin))
                {
                    break;
                }
                trim_input(order_nav);

                if (strcasecmp(order_nav, "B") == 0 || strcmp(order_nav, "0") == 0)
                {
                    break;
                }
                else if (strcasecmp(order_nav, "V") == 0)
                {
    // View Order Details
                    printf("  Enter No. to View Details [0-Back]: ");
                    char detail_input[32];
                    if (!fgets(detail_input, sizeof(detail_input), stdin))
                    {
                        continue;
                    }
                    trim_input(detail_input);
                    int detail_idx = atoi(detail_input);
                    if (detail_idx == 0)
                        continue;

                    TxNode *selected_order = get_user_order_by_index(stack, user->username, detail_idx);
                    if (!selected_order)
                    {
                        printf("  [ERROR] Invalid order number.\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }

                    while (1)
                    {
                        print_centered_header("ORDER DETAILS");
                        printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                        printf("  Order ID  : #%d\n", selected_order->order_id);
                        printf("  Date      : %s\n", selected_order->date);
                        printf("  Product   : %s\n", selected_order->item_title);
                        printf("  Seller    : %s\n", selected_order->seller);
                        printf("  Contact   : %s\n", selected_order->seller_contact);
                        printf("  Price     : PHP %.2f\n", selected_order->price);
                        printf("  Payment   : %s\n", selected_order->payment_method);
                        printf("  Status    : %s\n", selected_order->status);
                        printf("\n");

                        if (strcasecmp(selected_order->status, "Awaiting Confirmation") == 0)
                        {
                            printf("  [R] Mark as Received   [C] Cancel Order   [B] Back\n");
                        }
                        else if (strcasecmp(selected_order->status, "Pending") == 0)
                        {
                            printf("  [C] Cancel Order   [B] Back\n");
                        }
                        else
                        {
                            printf("  [B] Back to Buyer Menu\n");
                        }

                        printf("\n");
                        printf("================================================================================\n");
                        printf("  Enter choice [0-Back]: ");

                        char detail_choice[32];
                        if (!fgets(detail_choice, sizeof(detail_choice), stdin))
                        {
                            break;
                        }
                        trim_input(detail_choice);

                        if (strcasecmp(detail_choice, "B") == 0 || strcmp(detail_choice, "0") == 0)
                            break;

                        if (strcasecmp(detail_choice, "R") == 0)
                        {
                            if (strcasecmp(selected_order->status, "Awaiting Confirmation") != 0)
                            {
                                printf("  [ERROR] Only Awaiting Confirmation orders can be marked as Received.\n");
                                printf("  Press any key to continue...");
                                char back_input[32];
                                if (fgets(back_input, sizeof(back_input), stdin))
                                {
                                    trim_input(back_input);
                                }
                                continue;
                            }

                            print_centered_header("ORDER DETAILS");
                            printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                            printf("  Order ID  : #%d\n", selected_order->order_id);
                            printf("  Product   : %s\n", selected_order->item_title);
                            printf("  Seller    : %s\n", selected_order->seller);
                            printf("  Price     : PHP %.2f\n", selected_order->price);
                            printf("\n");
                            printf("  Confirm that you have received this item?\n");
                            printf("  [Y] Yes   [N] No\n");
                            printf("\n");
                            printf("================================================================================\n");
                            printf("  Enter choice [0-Back]: ");

                            char confirm_recv[16];
                            if (!fgets(confirm_recv, sizeof(confirm_recv), stdin))
                                continue;
                            trim_input(confirm_recv);

                            if (strcasecmp(confirm_recv, "Y") == 0)
                            {
                                strncpy(selected_order->status, "Completed", sizeof(selected_order->status) - 1);
                                selected_order->status[sizeof(selected_order->status) - 1] = '\0';
                                print_centered_header("ORDER DETAILS");
                                printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                                printf("  Order #%d has been marked as Received.\n", selected_order->order_id);
                                printf("  Transaction is now Completed.\n");
                                printf("  Thank you for using Trade Hub!\n");
                                printf("\n");
                                printf("================================================================================\n");
                                printf("  Press any key to continue...");
                                char back_input[32];
                                if (fgets(back_input, sizeof(back_input), stdin))
                                {
                                    trim_input(back_input);
                                }
                                break;
                            }
                            continue;
                        }
                        else if (strcasecmp(detail_choice, "C") == 0)
                        {
                            if (strcasecmp(selected_order->status, "Completed") == 0 ||
                                strcasecmp(selected_order->status, "Cancelled") == 0)
                            {
                                printf("  [ERROR] Only non-completed orders can be cancelled.\n");
                                printf("  Press any key to continue...");
                                char back_input[32];
                                if (fgets(back_input, sizeof(back_input), stdin))
                                {
                                    trim_input(back_input);
                                }
                                continue;
                            }

                            print_centered_header("CANCEL ORDER");
                            printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                            printf("  Order ID  : #%d\n", selected_order->order_id);
                            printf("  Product   : %s\n", selected_order->item_title);
                            printf("  Seller    : %s\n", selected_order->seller);
                            printf("  Price     : PHP %.2f\n", selected_order->price);
                            printf("  Status    : %s\n", selected_order->status);
                            printf("\n");
                            printf("  Are you sure you want to cancel this order?\n");
                            printf("  [Y] Yes   [N] No\n");
                            printf("\n");
                            printf("================================================================================\n");
                            printf("  Enter choice [0-Back]: ");

                            char confirm_cancel[16];
                            if (!fgets(confirm_cancel, sizeof(confirm_cancel), stdin))
                                continue;
                            trim_input(confirm_cancel);

                            if (strcasecmp(confirm_cancel, "Y") == 0)
                            {
                                strncpy(selected_order->status, "Cancelled", sizeof(selected_order->status) - 1);
                                selected_order->status[sizeof(selected_order->status) - 1] = '\0';
                                print_centered_header("ORDER CANCELLED");
                                printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                                printf("  Order #%d has been cancelled.\n", selected_order->order_id);
                                printf("\n");
                                printf("================================================================================\n");
                                printf("  Press any key to continue...");
                                char back_input[32];
                                if (fgets(back_input, sizeof(back_input), stdin))
                                {
                                    trim_input(back_input);
                                }
                                break;
                            }
                            continue;
                        }
                        else
                        {
                            printf("  [ERROR] Invalid choice. Enter R, C, B, or 0.\n");
                            printf("  Press any key to continue...");
                            char back_input[32];
                            if (fgets(back_input, sizeof(back_input), stdin))
                            {
                                trim_input(back_input);
                            }
                            continue;
                        }
                    }
                    continue;
                }
                else if (strcasecmp(order_nav, "R") == 0)
                {
    // Mark as Received
                    printf("  Enter No. to Mark as Received [0-Back]: ");
                    char recv_input[32];
                    if (!fgets(recv_input, sizeof(recv_input), stdin))
                    {
                        continue;
                    }
                    trim_input(recv_input);
                    int recv_idx = atoi(recv_input);
                    if (recv_idx == 0)
                        continue;

                    TxNode *selected_order = get_user_order_by_index(stack, user->username, recv_idx);
                    if (!selected_order)
                    {
                        printf("  [ERROR] Invalid order number.\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }

                    if (strcasecmp(selected_order->status, "Awaiting Confirmation") != 0)
                    {
                        print_centered_header("MY ORDERS");
                        printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                        printf("  Order #%d is currently %s.\n", selected_order->order_id, selected_order->status);
                        printf("  Only orders Awaiting Confirmation can be marked as Received.\n");
                        printf("\n");
                        printf("================================================================================\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }

                    print_centered_header("MY ORDERS");
                    printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                    printf("  Order ID  : #%d\n", selected_order->order_id);
                    printf("  Product   : %s\n", selected_order->item_title);
                    printf("  Seller    : %s\n", selected_order->seller);
                    printf("  Price     : PHP %.2f\n", selected_order->price);
                    printf("  Status    : %s\n", selected_order->status);
                    printf("\n");
                    printf("  Confirm that you have received this item?\n");
                    printf("  [Y] Yes   [N] No\n");
                    printf("\n");
                    printf("================================================================================\n");
                    printf("  Enter choice [0-Back]: ");

                    char confirm_recv[16];
                    if (!fgets(confirm_recv, sizeof(confirm_recv), stdin))
                    {
                        continue;
                    }
                    trim_input(confirm_recv);

                    if (strcasecmp(confirm_recv, "Y") == 0)
                    {
                        strncpy(selected_order->status, "Completed", sizeof(selected_order->status) - 1);
                        selected_order->status[sizeof(selected_order->status) - 1] = '\0';
                        print_centered_header("ORDER DETAILS");
                        printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                        printf("  Order #%d has been marked as Received.\n", selected_order->order_id);
                        printf("  Transaction is now Completed.\n");
                        printf("  Thank you for using Trade Hub!\n");
                        printf("\n");
                        printf("================================================================================\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (strcasecmp(order_nav, "C") == 0)
                {
    // Cancel Order
                    printf("  Enter No. to Cancel Order [0-Back]: ");
                    char cancel_input[32];
                    if (!fgets(cancel_input, sizeof(cancel_input), stdin))
                    {
                        continue;
                    }
                    trim_input(cancel_input);
                    int cancel_idx = atoi(cancel_input);
                    if (cancel_idx == 0)
                        continue;

                    TxNode *selected_order = get_user_order_by_index(stack, user->username, cancel_idx);
                    if (!selected_order)
                    {
                        printf("  [ERROR] Invalid order number.\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }

                    if (strcasecmp(selected_order->status, "Completed") == 0 ||
                        strcasecmp(selected_order->status, "Cancelled") == 0)
                    {
                        printf("  [ERROR] Only non-completed orders can be cancelled.\n");
                        printf("  Current status: %s\n", selected_order->status);
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }

                    print_centered_header("CANCEL ORDER");
                    printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                    printf("  Order ID  : #%d\n", selected_order->order_id);
                    printf("  Product   : %s\n", selected_order->item_title);
                    printf("  Seller    : %s\n", selected_order->seller);
                    printf("  Price     : PHP %.2f\n", selected_order->price);
                    printf("  Status    : %s\n", selected_order->status);
                    printf("\n");
                    printf("  Are you sure you want to cancel this order?\n");
                    printf("  [Y] Yes   [N] No\n");
                    printf("\n");
                    printf("================================================================================\n");
                    printf("  Enter choice [0-Back]: ");

                    char confirm_cancel[16];
                    if (!fgets(confirm_cancel, sizeof(confirm_cancel), stdin))
                    {
                        continue;
                    }
                    trim_input(confirm_cancel);

                    if (strcasecmp(confirm_cancel, "Y") == 0)
                    {
                        strncpy(selected_order->status, "Cancelled", sizeof(selected_order->status) - 1);
                        selected_order->status[sizeof(selected_order->status) - 1] = '\0';
                        print_centered_header("ORDER CANCELLED");
                        printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                        printf("  Order #%d has been cancelled.\n", selected_order->order_id);
                        printf("\n");
                        printf("================================================================================\n");
                        printf("  Press any key to continue...");
                        char back_input[32];
                        if (fgets(back_input, sizeof(back_input), stdin))
                        {
                            trim_input(back_input);
                        }
                        continue;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    printf("  [ERROR] Invalid choice. Enter V, R, C, or 0.\n");
                    printf("  Press any key to continue...");
                    char back_input[32];
                    if (fgets(back_input, sizeof(back_input), stdin))
                    {
                        trim_input(back_input);
                    }
                    continue;
                }
            }
        }
        else if (choice == 5)
        {
    // --- View Category Index ---
            while (1)
            {
                printf("\n");
                print_centered_header("BROWSE BY CATEGORY");
                printf("  Logged in as: %s (Buyer)\n\n", user->display_name);
                
                printf("  No.  Category    Total Listings    Description\n");
                printf("  [ 1] Uniform     29 item(s)        Blouse, Pants, Shoes, PE Uniform\n");
                printf("  [ 2] Supply      10 item(s)        Notebooks, Pens, Bags, Accessories\n");
                printf("  [ 3] Book        9  item(s)        Textbooks, Reviewers, Literature\n");
                printf("  [ 4] Other       10 item(s)        Miscellaneous Items\n");
                printf("\n");
                printf("  Total Categories : 4\n");
                printf("  Total Listings   : 58 item(s)\n");
                printf("\n");
                printf("================================================================================\n");
                printf("  Enter No. to Browse Category [0-Back]: ");

                char cat_choice[32];
                if (!fgets(cat_choice, sizeof(cat_choice), stdin))
                {
                    break;
                }
                trim_input(cat_choice);

                if (strcasecmp(cat_choice, "B") == 0 || strcmp(cat_choice, "0") == 0)
                {
                    break;
                }
                else if (atoi(cat_choice) >= 1 && atoi(cat_choice) <= 4)
                {
                    int cat_idx = atoi(cat_choice);
                    char category[50] = "";
                    switch (cat_idx)
                    {
                    case 1:
                        strcpy(category, "uniform");
                        break;
                    case 2:
                        strcpy(category, "supply");
                        break;
                    case 3:
                        strcpy(category, "book");
                        break;
                    case 4:
                        strcpy(category, "other");
                        break;
                    }

                    int subcat_chosen = 1;
                    while (subcat_chosen)
                    {
                        printf("\n");
                        print_centered_header("BROWSE BY CATEGORY");
                        printf("  Logged in as: %s (Buyer)\n\n", user->display_name);

                        if (strcasecmp(category, "uniform") == 0)
                        {
                            printf("  Category : Uniform (29 listings)\n\n");
                            printf("  No.  Sub-Category        Total Listings\n");
                            printf("  [ 1] School Uniform      3  item(s)\n");
                            printf("  [ 2] ROTC Uniform        10 item(s)\n");
                            printf("  [ 3] PE Uniform          8  item(s)\n");
                        }
                        else if (strcasecmp(category, "supply") == 0)
                        {
                            printf("  Category : Supply (10 listings)\n\n");
                            printf("  No.  Sub-Category        Total Listings\n");
                            printf("  [ 1] School Bag          2  item(s)\n");
                            printf("  [ 2] Writing Materials   5  item(s)\n");
                            printf("  [ 3] Other Supplies      3  item(s)\n");
                        }
                        else if (strcasecmp(category, "book") == 0)
                        {
                            printf("  Category : Book (9 listings)\n\n");
                            printf("  No.  Sub-Category        Total Listings\n");
                            printf("  [ 1] Textbooks           6  item(s)\n");
                            printf("  [ 2] Literature          3  item(s)\n");
                        }
                        else if (strcasecmp(category, "other") == 0)
                        {
                            printf("  Category : Other (10 listings)\n\n");
                            printf("  No.  Sub-Category        Total Listings\n");
                            printf("  [ 1] Electronics         4  item(s)\n");
                            printf("  [ 2] Accessories         6  item(s)\n");
                        }

                        printf("\n");
                        printf("================================================================================\n");
                        printf("  Enter No. to Browse Sub-Category [0-Back]: ");

                        char subcat_choice[32];
                        if (!fgets(subcat_choice, sizeof(subcat_choice), stdin))
                        {
                            subcat_chosen = 0;
                            break;
                        }
                        trim_input(subcat_choice);

                        if (strcasecmp(subcat_choice, "B") == 0 || strcmp(subcat_choice, "0") == 0)
                        {
                            subcat_chosen = 0;
                            continue;
                        }

                        int subcat_idx = atoi(subcat_choice);
                        if (subcat_idx <= 0)
                        {
                            printf("  [ERROR] Invalid choice. Enter a subcategory number or 0 to go back.\n");
                            continue;
                        }

                        char subcategory[100] = "";
                        if (strcasecmp(category, "uniform") == 0)
                        {
                            switch (subcat_idx)
                            {
                            case 1:
                                strcpy(subcategory, "School Uniform");
                                break;
                            case 2:
                                strcpy(subcategory, "ROTC Uniform");
                                break;
                            case 3:
                                strcpy(subcategory, "PE Uniform");
                                break;
                            default:
                                printf("  [ERROR] Invalid subcategory.\n");
                                continue;
                            }
                        }
                        else if (strcasecmp(category, "supply") == 0)
                        {
                            switch (subcat_idx)
                            {
                            case 1:
                                strcpy(subcategory, "School Bag");
                                break;
                            case 2:
                                strcpy(subcategory, "Notebook");
                                break;
                            case 3:
                                strcpy(subcategory, "Other");
                                break;
                            default:
                                printf("  [ERROR] Invalid subcategory.\n");
                                continue;
                            }
                        }
                        else if (strcasecmp(category, "book") == 0)
                        {
                            switch (subcat_idx)
                            {
                            case 1:
                                strcpy(subcategory, "Algebra Textbook");
                                break;
                            case 2:
                                strcpy(subcategory, "World History");
                                break;
                            default:
                                printf("  [ERROR] Invalid subcategory.\n");
                                continue;
                            }
                        }
                        else if (strcasecmp(category, "other") == 0)
                        {
                            switch (subcat_idx)
                            {
                            case 1:
                                strcpy(subcategory, "USB Flash Drive");
                                break;
                            case 2:
                                strcpy(subcategory, "Clipboard");
                                break;
                            default:
                                printf("  [ERROR] Invalid subcategory.\n");
                                continue;
                            }
                        }

                        display_items_by_subcategory(catalog, cart, q, stack, category, subcategory, user);
                    }
                }
                else
                {
                    printf("  [ERROR] Invalid choice. Enter a number 1-4 or 0.\n");
                }
            }
        }
        else if (choice == 6)
        {
    // --- Delete Account ---
            if (delete_account(users, user_ht, user, catalog) == 1)
            {
                break; /* Account deleted, return to startup */
            }
        }
        else if (choice == 7)
        {
    // --- Logout ---
            printf("Are you sure you want to logout? (y/n): ");
            char confirm[10];
            if (fgets(confirm, sizeof(confirm), stdin))
            {
                trim_input(confirm);
                if (confirm[0] == 'y' || confirm[0] == 'Y')
                {
                    break;
                }
            }
        }
        else if (choice == 0)
        {
    // --- Back to Main Menu ---
            break;
        }

    } while (choice != 0);
}
