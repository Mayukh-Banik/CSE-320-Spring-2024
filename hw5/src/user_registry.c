#include "Includes.h"
#include "user_registry.h"

#define MAX_USERS 2048 

typedef struct user_registry 
{
    USER* users[MAX_USERS];  
    pthread_mutex_t mutex;
    int num;
} 
USER_REGISTRY;

USER_REGISTRY* ureg_init(void) 
{
    USER_REGISTRY* ureg = calloc(1, sizeof(USER_REGISTRY));
    if (ureg == NULL) 
    {
        return NULL;
    }
    pthread_mutex_init(&ureg->mutex, NULL);
    ureg->num = 0;
    return ureg;
}

void ureg_fini(USER_REGISTRY* ureg) 
{
    if (ureg == NULL)
    {
        return;
    }
    pthread_mutex_lock(&ureg->mutex);
    for (int i = 0; i < ureg->num; i++) 
    {
        if (ureg->users[i] != NULL) 
        {
            user_unref(ureg->users[i], "ureg_fini");
        }
    }
    pthread_mutex_unlock(&ureg->mutex);
    pthread_mutex_destroy(&ureg->mutex);
    free(ureg);
}

USER* ureg_register(USER_REGISTRY* ureg, char* handle) 
{
    if (handle == NULL)
    {
        return NULL;
    }
    pthread_mutex_lock(&ureg->mutex);
    for (int i = 0; i < ureg->num; i++) 
    {
        if (ureg->users[i] != NULL && strcmp(user_get_handle(ureg->users[i]), handle) == 0) 
        {
            user_ref(ureg->users[i], "ureg_register");
            pthread_mutex_unlock(&ureg->mutex);
            return ureg->users[i];
        }
    }
    ureg->users[ureg->num] = user_create(handle);
    if (ureg->users[ureg->num] != NULL)
    {
        user_ref(ureg->users[ureg->num], "ureg_register");
        ureg->num++;
        pthread_mutex_unlock(&ureg->mutex);
        return ureg->users[ureg->num - 1];
    }
    pthread_mutex_unlock(&ureg->mutex);
    return NULL;
}

void ureg_unregister(USER_REGISTRY* ureg, char* handle) 
{
    if (ureg == NULL || handle == NULL)
    {
        return;
    }
    pthread_mutex_lock(&ureg->mutex);
    for (int i = 0; i < ureg->num; i++) 
    {
        if (ureg->users[i] != NULL && strcmp(user_get_handle(ureg->users[i]), handle) == 0) 
        {
            user_unref(ureg->users[i], "ureg_unregister");
            ureg->users[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&ureg->mutex);
}
