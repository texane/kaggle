#ifndef NN_HH_INCLUDED
# define NN_HH_INCLUDED


#include "fann.h"


struct table;


int nn_create_and_train(struct fann**, struct table&, struct table&);
void nn_destroy(struct fann*);
int nn_eval(struct fann*, struct table&, struct table&);
int nn_load(struct fann**, const char*);
int nn_save(struct fann*, const char*);



#endif // ! NN_HH_INCLUDED
