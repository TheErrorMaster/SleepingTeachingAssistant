#include <pthread.h>		//Create POSIX threads.
#include <time.h>				//Wait for a random time.
#include <unistd.h>			//Thread calls sleep for specified number of seconds.
#include <semaphore.h>	//To create semaphores
#include <stdlib.h>
#include <stdio.h>			//Input Output

pthread_t *Students;		//N threads running as Students.
pthread_t TA;						//Separate Thread for TA.

int ChairsCount = 0;
int CurrentIndex = 0;
int OfficeHours = 1;

/*TODO
//Declaration of Semaphores and Mutex Lock.
//Semaphores used:
//A semaphore to signal and wait TA's sleep.
//An array of 3 semaphores to signal and wait chair to wait for the TA.
//A semaphore to signal and wait for TA's next student.

//Mutex Lock used:
//To lock and unlock variable ChairsCount to increment and decrement its value.

 //hint: use sem_t and pthread_mutex_t
 */

sem_t Signal; // A semaphore to signal
sem_t TAs_Sleep; // A semaphore to TA's sleep
sem_t Wait_For_Chair[3]; // An array of 3 semaphores to signal and wait chair to wait for the TA.
pthread_mutex_t Lock_Chair_Count; //To lock and unlock variable ChairsCount to increment and decrement its value.
pthread_mutex_t OfficeHoursAccess; // To lock and unlock access to Office hours

//Declared Functions
void *TA_Activity(void *threadID);
void *Student_Activity(void *threadID);

// Main will intitialize the mutex_t objects and semaphores by creating threads, waiting for them to finish and then cleanup and end
int main(int argc, char* argv[])
{
	int number_of_students = 5;		//a variable taken from the user to create student threads.	Default is 5 student threads.
	int id = 0;
	srand(time(NULL));

	//*TODO
	//Initializing Mutex Lock and Semaphores.
	//hint: use sem_init() and pthread_mutex_init()
	// form linux documentation sem_init()has three parameters

	sem_init(&Signal, 0, 0);
	sem_init(&TAs_Sleep, 0, 0);
	sem_init(&Wait_For_Chair[0], 0, 0);
	sem_init(&Wait_For_Chair[1], 0, 0);
	sem_init(&Wait_For_Chair[2], 0, 0);

	// from the slides: pthread_mutex_init(&myMutex,NULL): initializes myMutex and sets its state to "unlocked".
	// pthread_mutex_lock(&myMutex):locks the mutex myMutex
	pthread_mutex_init(&Lock_Chair_Count, NULL); // Initialing pthread_mute_t Lock_Chair_Count

	if(argc<2)
	{
		printf("Number of Students not specified. Using default (5) students.\n");
	}
	else
	{
		printf("Number of Students specified. Creating %d threads.\n", number_of_students);
		number_of_students = atoi(argv[1]);
	}

	//Allocate memory for Students
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	/*TODO
	//Creating one TA thread and N Student threads.
	//hint: use pthread_create
	//Waiting for TA thread and N Student threads.
	//hint: use pthread_join
	*/

	// From the documentation pthread_create() has 4 parameters
	//int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void* (*start_routine) (void*), void* arg);
	// create attribute
pthread_attr_t attr; // set of thread attributes
pthread_attr_init(&attr); //Get the default attributes
		 pthread_create(&TA, &attr, TA_Activity, NULL);

		 for (id = 0; id < number_of_students; id++)
		 {
			 	// 3rd parameter is Student_Activity for Student
		 		pthread_create(&Students[id],&attr,Student_Activity,(void*)(long)id);
		 }


	//Waiting for TA thread and N Student threads.
  //hint: use pthread_join
	// From the documentation pthread_join() has 2 parameters
	// int pthread_join(pthread_t thread, void **retval);

		 // 3rd parameter is Student_Activity for Student
		 pthread_join(Students[0],NULL);
		 pthread_join(Students[1],NULL);
		 pthread_join(Students[2],NULL);

	//Initialize variable to signal that office hours are closed or open; O means closed
	OfficeHours = 0;
	pthread_join(TA, NULL);

	//Free allocated memory
	free(Students);
	return 0;
}


void *TA_Activity(void *threadID)
{
	//* TODO


	while(1)
	{
				//If office is closed, then students have been helped, we then unlock OfficeHoursAcces
				pthread_mutex_lock(&OfficeHoursAccess);
        if(OfficeHours == 0)
				{
					printf("All students have been helped. TA is going to sleep.\n");
					break;
        }
        pthread_mutex_unlock(&OfficeHoursAccess); // unlock

		sem_wait(&TAs_Sleep);//TA is currently sleeping.
		printf("TA is now awake to help student\n");// student wakes them up

		while(1)
		{
			//lock
			pthread_mutex_lock(&Lock_Chair_Count);

			if (ChairsCount == 0)
			{
				//if the chairs are empty, break out of the loop
				pthread_mutex_unlock(&Lock_Chair_Count);
				break;
			}

			//TA gets the next student from chairs
			sem_post(&Wait_For_Chair[CurrentIndex]);
			ChairsCount--;
			printf("Student left hallway chair, chairs remaining %d\n", 3 - ChairsCount);
			CurrentIndex = (CurrentIndex + 1) % 3;
			//unlock
			pthread_mutex_unlock(&Lock_Chair_Count);

			//TA is currently helping the student
			//hint: use sem_wait(); sem_post(); pthread_mutex_lock(); pthread_mutex_unlock()

			printf("*TA is helping the student \n");
			sleep(5); //sleep for 5 seconds
			sem_post(&Signal); //unlock Signal semaphore
			usleep(1000); //sleep for 1 millisecond
		}
	}
}

void *Student_Activity(void *threadID)
{
    //*TODO
		// variable to randomly sleep from anywhere between 1 and 5 seconds
	int homeworkTime;

	while(1)
	{
		printf("Student %ld is doing homework.\n", 1 + (long)threadID);
		homeworkTime = rand() % 5 + 1;
		sleep(homeworkTime);

		//Student  needs help from the TA
		printf("Student %ld needs help from the TA\n", 1 + (long)threadID);
		pthread_mutex_lock(&Lock_Chair_Count);
		int chairs = ChairsCount;
		pthread_mutex_unlock(&Lock_Chair_Count);

		if (chairs < 3)	// Student tried to sit on a chair.
		{
			if (chairs == 0) // wake up the TA
				sem_post(&TAs_Sleep);
			else // Student waits to go next.
				printf("Student %ld waits to go next.\n", 1 + (long)threadID);
			//lock
			pthread_mutex_lock(&Lock_Chair_Count);
			int index = (CurrentIndex + ChairsCount) % 3;
			ChairsCount++;
			printf("Student %ld waits in hallway, chairs available: %d\n",1 + (long)threadID, 3 - ChairsCount);
			pthread_mutex_unlock(&Lock_Chair_Count);
			//unlock

			//Student now leaves the chair and waits for thier turn.
			sem_wait(&Wait_For_Chair[index]);
			printf("Student %ld is getting help from the TA\n", 1 + (long)threadID);
			sem_wait(&Signal);
			printf("Student %ld left TA room\n", 1 + (long)threadID);
			//break statement so that each student is helped once, keeps looping otherwise
			break;
		}
		//If student didn't find any chair to sit on.
		//Student will return at another time
		//hint: use sem_wait(); sem_post(); pthread_mutex_lock(); pthread_mutex_unlock()
		//no chair to sit on, leave and come back later
		else
		{
				printf("If student didn't find any chair to sit on. Student %ld will return at another time\n", 1 + (long)threadID);
		}
	}
}
