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
  table::row_type::const_iterator iend = ref_in_table->rows[row].end();
  for (; ipos != iend; ++ipos, ++ins) *ins = *ipos;

  table::row_type::const_iterator opos = ref_out_table->rows[row].begin();
  table::row_type::const_iterator oend = ref_out_table->rows[row].end();
  for (; opos != oend; ++opos, ++outs) *outs = *opos;
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
#define CONFIG_HIDDEN_LAYER_COUNT 1
#define CONFIG_HIDDEN_NEURON_COUNT 20 // per layer
#define CONFIG_TOTAL_LAYER_COUNT (2 + CONFIG_HIDDEN_LAYER_COUNT)
  unsigned int layers[CONFIG_TOTAL_LAYER_COUNT];
  layers[0] = in_table.col_count;
  for (size_t i = 0; i < CONFIG_HIDDEN_LAYER_COUNT; ++i)
    layers[i + 1] = CONFIG_HIDDEN_NEURON_COUNT;
  layers[CONFIG_HIDDEN_LAYER_COUNT + 1] = out_table.col_count;

  *nn = fann_create_standard_array(CONFIG_TOTAL_LAYER_COUNT, layers);
  if (*nn == NULL) return -1;

  // create train set and train the network
  if (create_train_set(&train_data, in_table, out_table) == -1) goto on_error;

  fann_set_activation_function_hidden(*nn, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(*nn, FANN_LINEAR);
  fann_set_training_algorithm(*nn, FANN_TRAIN_RPROP);
  fann_set_learning_momentum(*nn, 0.4);
  fann_train_on_data(*nn, train_data, 300, 0, 0.01);

  fann_reset_MSE(*nn);
  fann_test_data(*nn, train_data);
  printf("Mean Square Error: %f\n", fann_get_MSE(*nn));

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

#if 1
int nn_eval(struct fann* nn, table& in_table, table& out_table)
{
  int error = -1;

  out_table.col_count = fann_get_num_output(nn);
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
#else
int nn_eval(struct fann* nn, table& in_table, table& out_table)
{
  out_table.col_count = fann_get_num_output(nn);
  out_table.row_count = in_table.row_count;
  out_table.rows.resize(in_table.row_count);

  for (size_t i = 0; i < out_table.row_count; ++i)
  {
    out_table.rows[i].resize(out_table.col_count);
    for (size_t j = 0; j < out_table.col_count; ++j)
      out_table.rows[i][j] = 1;
  }
  return 0;
}
#endif

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
