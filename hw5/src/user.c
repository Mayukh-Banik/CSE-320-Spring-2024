#include "Includes.h"
#include "user.h"
#include "debug.h"

typedef struct user
{
    sig_atomic_t referenceCount;
    char* handle;
    pthread_mutex_t mutex;
} USER;

USER* user_create(char* handle)
{
    USER* person = calloc(1, sizeof(USER));
    if (person == NULL)
    {
        return NULL;
    }
    if ((person->handle = strdup(handle)) == NULL) 
    {
        free(person);
        return NULL;
    }
    pthread_mutex_init(&person->mutex, NULL);
    person = user_ref(person, "Initializing user.");
    debug("Initializing user %p [%s]", person, person->handle);
    return person;
}

USER* user_ref(USER* user, char* why)
{
    if (user == NULL)
    {
        return NULL;
    }
    pthread_mutex_lock(&user->mutex);
    user->referenceCount++;
    debug("Increasing the reference count of %p [%s] to %d due to %s", user, user->handle, user->referenceCount, why);
    pthread_mutex_unlock(&user->mutex);
    return user;
}

void user_unref(USER* user, char* why)
{
    if (user == NULL)
    {
        return;
    }
    pthread_mutex_lock(&user->mutex);
    user->referenceCount--;
    debug("Decreasing the reference count of %p [%s] to %d due to %s", user, user->handle, user->referenceCount, why);
    if (user->referenceCount == 0)
    {
        debug("Freeing user %p [%s]", user, user->handle);
        free(user->handle);
        user->handle = NULL;
        pthread_mutex_unlock(&user->mutex); 
        pthread_mutex_destroy(&user->mutex);
        free(user);
        return;
    }
    pthread_mutex_unlock(&user->mutex);
}

char* user_get_handle(USER* user)
{
    if (user == NULL)
    {
        return NULL;
    }
    return user->handle;
}