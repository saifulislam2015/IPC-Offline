#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<unistd.h>


struct student
{
    int id;
    sem_t stsem;
    int password;
    int valid;
    struct student *ptr;
};

struct queue
{
    struct student *front;
    struct student *rear;
    sem_t empty;
    sem_t full;
    pthread_mutex_t lock;
    int count;
};

struct queue ACEq,Bq,B1q,acceptedq,requestq;

void init_queue(struct queue *q,int l)
{
    q->front = q->rear = NULL;
    q->count=0;
    sem_init(&q->empty,0,l);
	sem_init(&q->full,0,0);
	pthread_mutex_init(&q->lock,0);
	//printf("init");
}

void queuesize(struct queue *q)
{
    printf("\n Queue size : %d", q->count);
}

void enq(struct queue *q,struct student *s)
{
    sem_wait(&q->empty);
    pthread_mutex_lock(&q->lock);
    if (q->rear == NULL)
    {
        q->rear = s;
        //q->rear->ptr = NULL;
        //q->rear->id = data;
        //sem_init(&q->rear->stsem,0,0);
        //printf("\n Enqued value : %d", q->rear->id);
        q->front = q->rear;
    }
    else
    {
        //struct student *temp=(struct student *)malloc(1*sizeof(struct student));
        q->rear->ptr = s;
        //temp->id = data;
        //sem_init(temp->stsem,0,0);

        //temp->ptr = NULL;

        q->rear = s;
        //printf("\n Enqued value : %d", q->rear->id);
    }
    q->count++;
    pthread_mutex_unlock(&q->lock);
	sem_post(&q->full);
}


int search(struct queue *q,int a)
{
    pthread_mutex_lock(&q->lock);
    struct student *front1 = q->front;
    int c=0;

    if ((front1 == NULL) && (q->rear == NULL))
    {
        //printf("Queue is empty");
        return 0;
    }
    while (front1 != q->rear)
    {
        //printf("%d ", front1->id);
        if(front1->id==a) c++;
        front1 = front1->ptr;
    }
    if (front1 == q->rear){
        if(front1->id==a) c++;
    }
    pthread_mutex_unlock(&q->lock);
    return c;
}

struct student * deq(struct queue *q)
{
    sem_wait(&q->full);
    pthread_mutex_lock(&q->lock);
    //int x;
    struct student *front1 = q->front;
    struct student *temp;

    if (front1 == NULL)
    {
        //printf("\n Error: Trying to display elements from empty queue");
        return NULL;
    }
    else{
        if (front1->ptr != NULL)
        {
            front1 = front1->ptr;
            //printf("\n Dequed value : %d", q->front->id);
            //x=q->front->id;
            temp=q->front;
            //free(q->front);
            q->front = front1;
        }
        else
        {
            //printf("\n Dequed value : %d", q->front->id);
            //x=q->front->id;
            temp=q->front;
            //free(q->front);
            q->front = NULL;
            q->rear = NULL;
        }
        q->count--;
    }
    pthread_mutex_unlock(&q->lock);
    sem_post(&q->empty);
    //return x;
    return temp;
}


void remove_item(struct queue *q,int x)
{
    pthread_mutex_lock(&q->lock);
    struct student *temp,*prev,*temp1;
    if(search(q,x)==0) return;
    if(q->front==q->rear){
        free(q->front);
        q->front=q->rear=NULL;
    }
    else if(q->front->id==x) {
        temp1=q->front;
        q->front=q->front->ptr;
        free(temp1);

    }
    else{
        prev=q->front;
        temp=prev->ptr;
        while(temp->id!=x){
            prev=prev->ptr;
            temp=prev->ptr;
        }
        if(q->rear==temp) {
            q->rear=prev;
            q->rear->ptr=NULL;
        }
        prev->ptr=temp->ptr;
        free(temp);
    }
    pthread_mutex_unlock(&q->lock);
}

void display(struct queue *q)
{
    pthread_mutex_lock(&q->lock);
    struct student *front1 = q->front;

    if ((front1 == NULL) && (q->rear == NULL))
    {
        printf("Queue is empty");
        return;
    }
    while (front1 != q->rear)
    {
        printf("%d ", front1->id);
        front1 = front1->ptr;
    }
    if (front1 == q->rear)
        printf("%d\n", front1->id);
    pthread_mutex_unlock(&q->lock);
}

void * student_func(void *a)
{
    //sleep(1);
    int *b;
    b=(int *)a;
    struct student *temp=(struct student *)malloc(1*sizeof(struct student));
    temp->valid=0;
    temp->id=*b;
    //sem_init(&temp->stsem,0,0);

    enq(&ACEq,temp);
    printf("Student %d queued in ACEq\n",temp->id);

    sleep(1);

    enq(&B1q,temp);
    printf("Student %d queued in B1q\n",temp->id);

    sleep(1);

    //sem_wait(&temp->stsem);
    while(1){
        enq(&requestq,temp);
        //printf("Student %d queued in requestq\n",temp->id);
        if(temp->valid==1) {
            printf("\nStudent id:%d Password :%d\n\n",temp->id,temp->password);
            break;
        }
        else if(temp->valid>1) break;
        sleep(1);
    }
    //while(1);

}

void * ACE_func(void *x)
{
    struct student *temp;
    while(1){
        //char *c=x;
        //printf("%s ",c);
        temp=deq(&ACEq);
        enq(&Bq,temp);
        printf("ACE queued %d in Bq\n",temp->id);
        //display(&Bq);
        sleep(1);
    }
}

void * B_func(void *x)
{
    struct student *temp;
    int c;
    while(1){
        temp=deq(&B1q);
        while(1){
            c=search(&Bq,temp->id);
            if(c==1){    //remove from queuq
                //remove_item(&Bq,temp->id);
                temp->password=rand()%100;
                enq(&acceptedq,temp);
                printf("B queued %d in acceptedq\n",temp->id);
                break;
            }
            else if(c>1){
                printf("------------Duplicate %d\n",temp->id);
                temp->valid=2;
                break;
            }
            else sleep(1);
        }

    }
}

void * D_func(void *x)
{
    struct student *temp;
    int c;
    while(1){
        temp=deq(&requestq);
        //printf("D dequeued %d from requestq\n",temp->id);
        //display(&acceptedq);
        c=search(&acceptedq,temp->id);
        if(c==1){
            printf("D granted %d\n",temp->id);
            //sem_post(&temp->stsem);
            temp->valid=1;
        }

    }
}

int main()
{
    int i,t;
    init_queue(&ACEq,10);
    init_queue(&Bq,500);
    init_queue(&B1q,1);
    init_queue(&acceptedq,500);
    init_queue(&requestq,500);
    pthread_t threads[30];
    pthread_t A;
    pthread_t C;
    pthread_t E;
    pthread_t B;
    pthread_t D;

    pthread_create(&D,NULL,D_func,NULL);

    pthread_create(&B,NULL,B_func,NULL);

    char *a="A";
    pthread_create(&A,NULL,ACE_func,(void *)a);
    //pthread_join(A,NULL);
    char *b="E";
    pthread_create(&E,NULL,ACE_func,(void *)b);
    char *c="C";
    pthread_create(&C,NULL,ACE_func,(void *)c);



    int st[30];
    for(i=0;i<40;i++) st[i]=i;
    st[0]=0;
    st[1]=0;
    st[25]=5;
    st[5]=5;
    st[15]=5;
    for(t=0;t<30;t++){
        pthread_create(&threads[t],NULL,student_func,(void *)&st[t]);
        pthread_join(threads[t],NULL);
    }

    //sleep(30);
    //display(&Bq);
    //sleep(5);
    //display(&acceptedq);
    //pthread_exit(NULL);
    //while(1);
    return 0;
}


