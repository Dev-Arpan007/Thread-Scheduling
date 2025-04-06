#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>

int timer=0, timeQ, context;
int compTime[20]={0}, arrival[20]={0}, burstArr[20]={0}, avgTAT[20]={0}, avgWait[20]={0};
int endorder[20];
int ind=0, flag_CT=1, counter_flag=0, n;
int context_count=0;

sem_t s, s1;

typedef struct Node {
    int thdNum;
    int arrTime;
    int burstTime;
    int nextThdNum;
    struct Node* next;
} Node;

Node* head=NULL;

void enqueueRQ(Node** head, int atime, int burst, int num) {
    Node* temp=(*head);
    Node* thdnode = (Node*)malloc(sizeof(Node));
    thdnode->arrTime=atime;
    thdnode->burstTime=burst;
    thdnode->thdNum=num;
    
    if(*head == NULL) {
        (*head) = thdnode;
        (*head)->next=NULL;
        (*head)->nextThdNum=-1;
        return;
    }
    else if((((*head)->arrTime)>atime)||((((*head)->arrTime)==atime) && (((*head)->thdNum)>num))) {
        thdnode->next=(*head);
        thdnode->nextThdNum=(*head)->thdNum;
        (*head)=thdnode;
    }
    else {
        int flag=0;
        while(temp->next!=NULL) {
            int x = temp->next->arrTime;
            if((x>atime)||((x==atime) && ((temp->next->thdNum)>num))) {
                thdnode->next=temp->next;
                thdnode->nextThdNum=temp->next->thdNum;
                temp->next=thdnode;
                temp->nextThdNum=thdnode->thdNum;
                flag=1;
                break;
            }
            temp=temp->next;
        }
        if(flag==0) {
            temp->next=thdnode;
            thdnode->next=NULL;
            temp->nextThdNum=thdnode->thdNum;
            thdnode->nextThdNum=-1;
        }
    }
}

void enqueueRQ1(Node** head, int atime, int burst, int num) {
    Node* temp=(*head);
    Node* thdnode = (Node*)malloc(sizeof(Node));
    thdnode->arrTime=atime;
    thdnode->burstTime=burst;
    thdnode->thdNum=num;
    
    if(*head == NULL) {
        (*head) = thdnode;
        (*head)->next=NULL;
        (*head)->nextThdNum=-1;
        return;
    } 
    else if(((*head)->arrTime)>atime) {
        thdnode->next=(*head);
        thdnode->nextThdNum=(*head)->thdNum;
        (*head)=thdnode;
    }
    else {
        int flag=0;
        while(temp->next!=NULL) {
            int x= temp->next->arrTime;
            if((x>atime)) {
                thdnode->next=temp->next;
                thdnode->nextThdNum=temp->next->thdNum;
                temp->next=thdnode;
                temp->nextThdNum=thdnode->thdNum;
                flag=1;
                break;
            }
            temp=temp->next;
        } 
        if(flag==0) {
            temp->next=thdnode;
            thdnode->next=NULL;
            temp->nextThdNum=thdnode->thdNum;
            thdnode->nextThdNum=-1;
        }
    }
}

void removeRQ(Node** head) {
    if((*head)==NULL)
        return;
    Node* temp1=(*head);
    (*head)=(*head)->next;
    temp1->next=NULL;
    free(temp1);
    return;
}

void displayRQ(Node* head) {
    printf("\n");
    Node* temp = head;
    while(temp!=NULL) {
        int n1=temp->thdNum, n2=temp->arrTime, n3=temp->burstTime, n4=temp->nextThdNum;
        printf("[T%d,%d,%d,%d] --->", n1, n2, n3, n4);
        temp=temp->next;
    }
    printf("\n");
}

void* threadfunction(void* arg) {
    int* arr=(int*)arg;
    if((arr[3]+1)==n)
        counter_flag=1;
    
    while(counter_flag==0);
    
    int* retval = (int*)malloc(sizeof(int));
    *retval=arr[0];
    int num=arr[0];
    int burst=arr[1];
    int tid=arr[2];
    
    while(burst>0) {
        if(head!=NULL) {
            if(num==(head->thdNum)) {
                if(timer<(head->arrTime)) {
                    sem_wait(&s);
                    int m;
                    for(m=1;m<=((head->arrTime)-timer);m++) {
                        printf("i ");
                        context_count++;
                    }
                    timer=head->arrTime;
                }
                if(flag_CT==1) {
                    flag_CT=0;
                }
                else {
                    timer+=context;
                    int p;
                    for(p=1;p<=context;p++) {
                        printf("x ");
                        context_count++;
                    }
                }
                if(timer>=(head->arrTime)) {
                    int m;
                    for(m=1; m<=(timeQ*10);m++) {
                        if(m%10==0) {
                            timer++;
                            burst--;
                            compTime[num]=timer;
                            printf("T%d ", num);
                            if(burst==0) {
                                endorder[ind]=tid;
                                ind++;
                                break;
                            }
                        }
                    }
                    if(burst==0) {
                        removeRQ(&head);
                        sem_post(&s);
                        break;
                    }
                    else {
                        enqueueRQ1(&head, timer, burst, num);
                        removeRQ(&head);
                        sem_post(&s);
                    }
                }
            }
        }
    }
    free(arr);
    while(head!=NULL);
    pthread_exit((void*)retval);
}

void printChart(int arrival[], int burst[], int comp[]) {
    int k=1,wait;
    while(comp[k]!=0) {
        wait=((comp[k]-arrival[k])-burst[k]);
        printf("Thread%d \t Arrival = %d\t Burst = %d\t Completion = %d\t TAT = %d\t Wait = %d\n",k, arrival[k], burst[k], comp[k], (comp[k]-arrival[k]), wait);
        avgTAT[k]=(comp[k]-arrival[k]);
        avgWait[k]=wait;
        k++;
    }
}

void printAVG() {
    int p=1, sumwait=0, sumtat=0;
    float avgwait, avgtat, idle;
    while(p<=n) {
        sumwait+=avgWait[p];
        sumtat+=avgTAT[p];
        p++;
    }
    avgwait=sumwait/(float)n;
    avgtat=sumtat/(float)n;
    printf("Average Wait = %f, Average TAT = %f\n", avgwait, avgtat);
    idle=((float)context_count/timer)*100; 
    printf("CPU idle percentage = %f", idle);
}

int main() {
    int i,num, atime, burst,j,k=0;
    pthread_t t[20];
    int* ret;
    sem_init(&s,0,1);
    
    printf("enter the number of threads to perform Round-Robin Scheduling: \n");
    scanf("%d",&n);
    
    for(i=1;i<=n;i++) {
        printf("Enter arrival time & burst time for thread%d respectively\n", i);
        num=i;
        scanf("%d%d",&atime, &burst);
        enqueueRQ(&head, atime, burst, num);
        displayRQ(head);
        burstArr[num]=burst;
        arrival[num]=atime;
    }
    
    printf("enter time quantum and context-switch time respectively:\n");
    scanf("%d%d",&timeQ, &context);
    
    int count=0;
    Node* check=head;
    while(count<n) {
        int* t_arr = malloc(4*sizeof(int));
        t_arr[0]=check->thdNum;
        t_arr[1]=check->burstTime;
        t_arr[2]=k;
        t_arr[3]=count;
        
        if((pthread_create(&t[k], NULL, threadfunction, (void*)t_arr))!=0) {
            perror("Thread Creation failed...\n");
        }
        else {
            printf("thread%d created successfully\n",t_arr[0]);
            k++;
            check=check->next;
            count++;
        }
    }
    
    int l;
    for(l=0;l<n;l++) {
        if(pthread_join(t[endorder[l]], (void**)&ret)!=0) {
            printf("\nthread%d termination unsuccessfull\n", (*ret));
        } 
        else {
            printf("\nthreadid %d successfully terminated", (*ret)); 
            free(ret);
        }
    }
    
    sem_destroy(&s);
    printf("\n");
    printChart(arrival, burstArr, compTime);
    printf("\n");
    printAVG();
    return 0;
}