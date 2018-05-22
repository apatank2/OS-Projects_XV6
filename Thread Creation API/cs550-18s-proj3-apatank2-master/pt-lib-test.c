#include "types.h"
#include "user.h"
#include "procThread.h"

int count = 0;

void usage(char *argv[])
{
    printf(1, "Usage: %s test-case-# (1-8)\n", argv[0]);
}


void change_local_var(int tid)
{
    int a = 100;
    
    printf(1, "In %s: before: a = %d\n", tid == 0 ? "CHILD thread":"MAIN thread", a);
    a++;
    printf(1, "In %s: after: a = %d\n", tid == 0 ? "CHILD thread":"MAIN thread", a);
}

int main(int argc, char *argv[])
{ 
    if (argc < 2)
    {
        usage(argv);
        exit();
    }

    switch (argv[1][0])
    {

        case '1':
        {
            /***************************** TEST CASE 1 *********************************
            What to test --> Modifying global variable: child threads first, main thread second, 
                             without intermediate outputs
            
            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 1 -------------------- 
            Printing in MAIN thread: count=3 
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            count = 0;

            printf(1, "-------------------- TEST CASE 1 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                count++;
 //printf(1, "1.count - %d. \n", count);
                return 0;    
            }

            sleep(50);
//printf(1, "2.count - %d. \n", count);
            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                count++;
//printf(1, "3.count - %d. \n", count);
                return 0;    
            }

            sleep(50);

            count++;
            printf(1, "Printing in MAIN thread: count=%d \n", count);

            break;
        }

    
        case '2':
        {
            /***************************** TEST CASE 2 *********************************
            What to test --> Modifying global variable: child threads first, main thread second, 
                             with intermediate outputs

            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 2 -------------------- 
            Printing in CHILD thread: counter=1 
            Printing in MAIN thread: thread 1 (tid = x) created 
            Printing in CHILD thread: counter=2 
            Printing in MAIN thread: thread 2 (tid = x) created 
            Printing in MAIN thread: count=3              
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            count = 0;

            printf(1, "-------------------- TEST CASE 2 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                count++;
                printf(1, "Printing in CHILD thread: counter=%d \n", count);
                return 0;    
            }

            sleep(50);
            printf(1, "Printing in MAIN thread: thread 1 (tid = %d) created \n", tid_1);

            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                count++;
                printf(1, "Printing in CHILD thread: counter=%d \n", count);
                return 0;    
            }

            sleep(50);
            printf(1, "Printing in MAIN thread: thread 2 (tid = %d) created \n", tid_2);

            count++;
            printf(1, "Printing in MAIN thread: count=%d \n", count);

            break;
        }

        case '3':
        {
            /***************************** TEST CASE 3 *********************************
            What to test --> Modifying global variable: main thread first, child threads second, 
                             without intermediate outputs

            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 3 -------------------- 
            Printing in CHILD thread: counter=3 
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            count = 0;

            printf(1, "-------------------- TEST CASE 3 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                sleep(50);
                count++;
                return 0;    
            }

            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                sleep(150);
                count++;
                printf(1, "Printing in CHILD thread: counter=%d \n", count);
                return 0;    
            }

            count++;

            break;
        }


        case '4':
        {
            /***************************** TEST CASE 4 *********************************
            What to test --> Modifying global variable: main thread first, child threads second, 
                             with intermediate outputs

            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 4 -------------------- 
            Printing in MAIN thread: thread 1 (tid = x) created 
            Printing in MAIN thread: thread 2 (tid = x) created 
            Printing in MAIN thread: count=1 
            Printing in CHILD thread: counter=2 
            Printing in CHILD thread: counter=3 
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            count = 0;

            printf(1, "-------------------- TEST CASE 4 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                sleep(50);
                count++;
                printf(1, "Printing in CHILD thread: counter=%d \n", count);
                return 0;    
            }

            printf(1, "Printing in MAIN thread: thread 1 (tid = %d) created \n", tid_1);

            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                sleep(150);
                count++;
                printf(1, "Printing in CHILD thread: counter=%d \n", count);
                return 0;    
            }

            printf(1, "Printing in MAIN thread: thread 2 (tid = %d) created \n", tid_2);

            count++;
            printf(1, "Printing in MAIN thread: count=%d \n", count);

            break;
        }


        case '5':
        {
            /***************************** TEST CASE 5 *********************************
            What to test --> Modifying a local variable that is created after new thread is formed
            
            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 5 -------------------- 
            In CHILD thread: before: a = 100
            In CHILD thread: after: a = 101
            In MAIN thread: before: a = 100
            In MAIN thread: after: a = 101
            ****************************************************************************/
            int tid = 0;

            printf(1, "-------------------- TEST CASE 5 -------------------- \n");

            // -----
            tid = procThread_create();
            if (tid < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }

            if (tid > 0) sleep (100);

            change_local_var(tid); // both the main thread and child thread will execute this function 

            break;
        }

        
        case '6':
        {
            /***************************** TEST CASE 6 *********************************
            What to test --> Modifying a local variable that is created before new thread is formed
            
            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 6 -------------------- 
            In CHILD thread: before: b = 200
            In CHILD thread: after: b = 201
            In MAIN thread: before: b = 200
            In MAIN thread: after: b = 201
            ****************************************************************************/
            int tid = 0, b = 200;

            printf(1, "-------------------- TEST CASE 6 -------------------- \n");

            // -----
            tid = procThread_create();
            if (tid < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }

            if (tid > 0) sleep (100);

            // both the main thread and child thread will execute the following code:
            printf(1, "In %s: before: b = %d\n", tid == 0 ? "CHILD thread":"MAIN thread", b);
            b++;
            printf(1, "In %s: after: b = %d\n", tid == 0 ? "CHILD thread":"MAIN thread", b);

            break;
        }

        case '7':
        {
            /***************************** TEST CASE 7 *********************************
            What to test --> Joining terminated threads to get their return/exit values
            
            Expected output (shell prompt mixing in is okay) -->

            -------------------- TEST CASE 7 -------------------- 
            Printing in MAIN thread: thread 1 returning 0x100 
            Printing in MAIN thread: thread 2 returning 0x200 
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            int ret_1 = 0, ret_2 = 0;

            printf(1, "-------------------- TEST CASE 7 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                return 0x100;    
            }

            sleep(50);

            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                procThread_exit(0x200);    
            }

            sleep(50);

            procThread_join(tid_1, &ret_1);
            procThread_join(tid_2, &ret_2);
            
            printf(1, "Printing in MAIN thread: thread 1 returning 0x%x \n", ret_1);
            printf(1, "Printing in MAIN thread: thread 2 returning 0x%x \n", ret_2);        

            break;
        }


        case '8':
        {
            /***************************** TEST CASE 8 *********************************
            What to test --> Joining terminated threads and free the stack space
            
            Expected output (shell prompt mixing in is okay) (the difference should be
            about 1 byte) -->

            -------------------- TEST CASE 8 -------------------- 
            &a = 0xBF73
            &a = 0xBF72            
            ****************************************************************************/
            int tid_1 = 0, tid_2 = 0;
            int ret_1 = 0, ret_2 = 0;            

            printf(1, "-------------------- TEST CASE 8 -------------------- \n");

            // -----
            tid_1 = procThread_create();
            if (tid_1 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_1 == 0) // child thread 1
            {
                char a = 0;
                printf(1, "&a = 0x%x\n", &a);
                return 0;    
            }

            sleep(50);

            procThread_join(tid_1, &ret_1);

            // -----
            tid_2 = procThread_create();
            if (tid_2 < 0)
            {
                printf(1, "Thread creation failed. \n");
                exit();
            }
            else if (tid_2 == 0) // child thread 2
            {
                char a = 0;
                printf(1, "&a = 0x%x\n", &a);
                return 0;
            }

            procThread_join(tid_2, &ret_2);
            
            break;
        }



        default:
            printf(1, "No such test case (%c).\n", argv[1][0]);
            usage(argv);
            break;
    }

    exit();
}



