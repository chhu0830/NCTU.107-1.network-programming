#include <string.h>
#include "user.h"

void reset_user(struct USER *user, int id)
{
    memset(user, 0, sizeof(struct USER));
    user->id = id;
}
