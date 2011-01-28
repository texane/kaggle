#include <stdlib.h>
#include "fann.h"
#include "nn.hh"
#include "table.hh"


// callback needs those
static table* ref_in_table;
static table* ref_out_table;

static void on_data
(unsigned int row, unsigned int, unsigned int, fann_type* ins, fann_type* outs)
{
  // row the row index
  // ins the input values
  // outs the output values

  table::row_type::const_iterator ipos = ref_in_table->rows[row].begin();
  table::row_type::const_iterator opos = ref_out_table->rows[row].begin();
  table::row_type::const_iterator end = ref_in_table->rows[row].end();
  for (; ipos != end; ++ipos, ++opos, ++ins, ++outs)
  {
    *ins = *ipos;
    *outs = *opos;
  }
}

static int create_train_set
(struct fann_train_data** train_set, table& in_table, table& out_table)
{
  ref_in_table = &in_table;
  ref_out_table = &out_table;

  *train_set = fann_create_train_from_callback
    (in_table.row_count, in_table.col_count, out_table.col_count, on_data);

  return (*train_set == NULL) ? -1 : 0;
}

int nn_create_and_train(struct fann** nn, table& in_table, table& out_table)
{
  struct fann_train_data* train_data = NULL;
  int error = -1;

  // create the network
  unsigned int layers[3];
  layers[0] = in_table.col_count;
  layers[1] = 10;
  layers[2] = out_table.col_count;

  *nn = fann_create_standard_array(3, layers);
  if (*nn == NULL) return -1;

  // create train set and train the network
  if (create_train_set(&train_data, in_table, out_table) == -1) goto on_error;

  fann_set_training_algorithm(*nn, FANN_TRAIN_INCREMENTAL);
  fann_set_learning_momentum(*nn, 0.4);
  fann_train_on_data(*nn, train_data, 3000, 0, 0.001);

  // success
  error = 0;

 on_error:
  if (train_data != NULL) fann_destroy_train(train_data);
  return error;
}

void nn_destroy(struct fann* nn)
{
  fann_destroy(nn);
}

int nn_eval(struct fann* nn, table& in_table, table& out_table)
{
  int error = -1;

  // hack
  out_table.col_count = 1;
  out_table.row_count = in_table.row_count;
  out_table.rows.resize(in_table.row_count);

  fann_type* const inputs = (fann_type*)malloc
    (in_table.col_count * sizeof(fann_type));
  if (inputs == NULL) return -1;

  for (size_t i = 0; i < in_table.row_count; ++i)
  {
    for (size_t j = 0; j < in_table.col_count; ++j)
      inputs[j] = in_table.rows[i][j];

    fann_type* const outputs = fann_run(nn, inputs);
    if (outputs == NULL) goto on_error;

    out_table.rows[i].resize(out_table.col_count);
    for (size_t j = 0; j < out_table.col_count; ++j)
      out_table.rows[i][j] = outputs[j];
  }

  // success
  error = 0;

 on_error:
  free(inputs);
  return error;
}

int nn_load(struct fann** nn, const char* filename)
{
  *nn = fann_create_from_file(filename);
  return 0;
}

int nn_save(struct fann* nn, const char* filename)
{
  fann_save(nn, filename);
  return 0;
}
