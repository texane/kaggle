function data = nn_csvread(filename)
  # remove the very first list
  data = csvread(filename);
  data(1,:) = [];
endfunction

function subset = nn_get_training_subset(data, cols)
  rows = find(data(1:size(data)(1),2)<10);
  subset = data(rows, [3 cols]); # keep the col 3 result
endfunction

function [train_set, res_set] = nn_get_training_subset2(data, cols)
  rows = find(data(1:size(data)(1),2)<10);
  train_set = data(rows, cols);
  res_set = data(rows, 3);
endfunction

function [test_set, res_set] = nn_get_testing_set(data, cols, tid)
  if nargin == 2 tid = 100; end # random
  # get a subset for testing
  rows = find(data(:,1) == tid);
  res_set = data(rows,3);
  test_set = data(rows,cols);
endfunction

function ti = nn_get_eval_set(ds)
  # ti the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  # transposed
  ti = ds(1:size(ds)(1), 4:ncols);
  ti = ti';
endfunction

function train_set = nn_get_training_set(data, cols, tids)
  # the first col contains the results
  train_rows = [];
  for tid = tids
    rows = find(data(:,1)==tid);
    train_rows = [train_rows rows'];
  end
  train_set = data(train_rows, [3 cols]);
endfunction

function [nn, mean, std] = nn_train(data)
  # data the input data with results in first col

  # get the transposed sets
  to = data(:,1)';
  ti = data(:,2:size(data)(2))';

  # standardize the inputs
  [in_foo, in_mean, in_std] = prestd(ti);

  # hidden and output layers neurons count
  # nn_counts = [10, 1];
  nn_counts = [2, 1];
  # nn_counts = [1, 1];
  # layers transfer function
  nn_funcs = {"tansig", "purelin"};

  # instanciate the network
  net = newff(min_max(in_foo), nn_counts, nn_funcs, "trainlm", "mse");
  # net = newff(min_max(in_foo), nn_counts, nn_funcs, "trainrp", "mse");
  net.trainParam.epochs = 50;
  net.trainParam.mem_reduc = 2;
  net.trainParam.show = 10;

  # disable plotting
  # net.trainParam.show = NaN;

  # layer weights
  net.IW{1,1}(:) = 1.5;
  net.LW{2,1}(:) = 0.5;
  net.b{1,1}(:) = 1.5;
  net.b{2,1}(:) = 0.5;

  # define validation data new, for matlab compatibility
  VV.P = ti;
  VV.T = to;
  # standardize also the validate data
  VV.P = trastd(VV.P, in_mean, in_std);

  # train the network
  [nn] = train(net, in_foo, to, [], [], VV);
  mean = in_mean;
  std = in_std;
endfunction

function outs = nn_sim(nn, mean, std, input)
  # output the proba
  # nn the neural network from nn_train
  # data the input data
  data = input'; # transposed
  [simout] = sim(nn, trastd(data, mean, std));
  rows = size(data)(2);
  outs = zeros(rows, 1);
  for i = 1:rows
    outs(i) = simout(1, i);
  end
endfunction

function outs = nn_eval(nn, mean, std, input)
  # nn the neural network from nn_train
  # simin the simulation input
  data = input'; # transposed
  [simout] = sim(nn, trastd(data, mean, std));
  rows = size(data)(2);
  outs = zeros(rows, 1);
  for i = 1:rows
    value = simout(1,i);
    if value < 0.5 value = 0.0;
    else value = 1.0; end
    outs(i) = value;
  end
endfunction

function score = nn_score(nn, mean, std, data, res)
  # nn the trained nn
  # data the data set
  # res the expected results
  out = nn_eval(nn, mean, std, data);
  score = 0;
  for i = 1:size(out)(1)
    value = out(i);
    if value == res(i) score = score + 1; end
  end
  score = score / size(out)(1);
endfunction

function nn_submit(td, res)
  # td the test set
  # res the results
  file = fopen("/tmp/fu.csv", "wt");
  fwrite(file, "TrialID,ObsNum,Prediction\n");
  for i = 1:size(res)(1)
    fprintf(file, "%d,%d,%d\n", td(i,1), td(i,2), res(i));
  end
  fclose(file);
endfunction

function choice = nn_fortune(weights)
  accumulation = cumsum(weights);
  p = rand() * accumulation(end);
  chosen_index = -1;
  for index = 1 : length(accumulation)
    if (accumulation(index) > p)
      chosen_index = index;
      break;
    end
  end
  choice = chosen_index;
endfunction

function nn_choose_net(data)
  # use a genetic algorithm to find the sample to use

  # number of generation
  gen_count = 10;

  # population count per generation
  pop_per_gen = 20;

  # per population sample count
  samples_per_pop = 10;

  # current sample weights. start with equiprobability.
  sample_weights = ones(1, 33);
  # sample_weights([1 2]) = [0 0];
  sample_weights([1 2 3]) = [0 0 0];

  # foreach generation
  for i = 1:gen_count

    printf("\n\n***** generation %d\n\n", i);

    # initialize generation samples and scores
    gen_samples = zeros(pop_per_gen, samples_per_pop);
    gen_scores = zeros(1, pop_per_gen);

    # foreach population
    for j = 1:pop_per_gen

      # non uniform sample random generation
      # sample_dist contains the distribution
      pop_samples = zeros(1,samples_per_pop);
      sample_dist = sample_weights;
      for k = 1:samples_per_pop
	sample = nn_fortune(sample_dist);
	pop_samples(1,k) = sample;
	sample_dist(sample) = 0; # do not choose twice
      end

      # get the training data set and train the nn
      train_set = nn_get_training_subset(data, pop_samples);
      [nn, mean, std] = nn_train(train_set);

      # get the evaluation and result data sets
      rows = find(data(:,2) == 2);
      resize(rows, 1000);
      eval_set = data(1:size(rows)(1), pop_samples);
      res_set = data(1:size(rows)(1), 3);

      # add the scores and pop_samples to gen_samples
      gen_samples(j,:) = pop_samples;
      gen_scores(j) = nn_score(nn, mean, std, eval_set, res_set);

    end # foreach(population)

    # sort the scores
    [sort_val, sort_ind] = sort(gen_scores, 'ascend');

    # update sample weights
    for i = 1:size(sort_ind)(2)
      for j = 1:samples_per_pop # samples for this score
	k = gen_samples(sort_ind(i), j);
	sample_weights(1, k) += i;
      end
    end

  end # foreach(generation)

  # print the resulting weights
  [sort_val, sort_ind] = sort(sample_weights, 'descend');
  for i = 1:size(sample_weights)(2)
    printf("%f %f\n", sort_ind(i), sort_val(i));
  end

endfunction


function tids = gen_tids(tid_count)
  # max number of tids
  tid_max = 510;

  # clear the tids
  tids = [];

  # generate tid_count distinct tids 
  while tid_count
    tid = floor(rand() * tid_max);

    # add if not already present
    is_found = 0;

    for j = tids
      if j == tid
	is_found = 1;
	break ;
      end
    end

    if is_found == 0
      tid_count = tid_count - 1;
      tids = [tids tid];
    end

  end # tid_count
endfunction


function [data, nn, mean, std, cols] = nn_create_net(tids)
  if nargin == 0
    tids = gen_tids(100);
  end

  cols = [4:33];
  data = nn_csvread('../../data/fordTrain.csv');
  train_set = nn_get_training_set(data, cols, tids);
  [nn, mean, std] = nn_train(train_set);
endfunction


function nn_doit(data, nn, mean, std, cols)
  if nargin == 0
    [data, nn, mean, std, cols] = nn_create_net();
  end

  for tid = 300:310
    [test_set, res_set] = nn_get_testing_set(data, cols, tid);
    score = nn_score(nn, mean, std, test_set, res_set);
    printf("%d %f\n", tid, score);
  end
endfunction


function nn_do_submit(data, nn, mean, std, cols)
  submission_data = nn_csvread('../../data/fordTest.csv');
  [submission_set, res_set] = nn_get_submission_set_deriv(submission_data, cols);
  submission_res = nn_eval(nn, mean, std, submission_set);
  nn_submit(submission_data, submission_res);
endfunction


function scores = nn_do_score(data, nn, mean, std, cols, tids)
  scores = [];
  i = 1;
  for tid = tids
    #[test_set, res_set] = nn_get_testing_set_deriv_only(data, cols, tid);
    [test_set, res_set] = nn_get_testing_set_deriv(data, cols, tid);
    #[test_set, res_set] = nn_get_testing_set(data, cols, tid);
    scores(i) = nn_score(nn, mean, std, test_set, res_set);
    i = i + 1;
  end

  # post filter
  z = find(scores == 0);
  if (length(z) == 0) return ; end

  for i = 1:length(z)
    [zero_data, zero_res] = nn_get_testing_set_deriv(data, cols, tids(z(i)));
    printf("tid: %d\n", tids(z(i)));
    subplot(length(z), 1, i); plot(zero_res(:, 1));
  end

endfunction


function auc = nn_do_auc(data, nn, mean, std, cols, tids)
  predicted = [];
  actual = [];

  for tid = tids
    [test_set, res_set] = nn_get_testing_set_deriv(data, cols, tid);
    actual = [ actual res_set' ];
    out = nn_eval(nn, mean, std, test_set);
    predicted = [ predicted out' ];
  end
  auc = SampleError(predicted, actual, 'AUC');
endfunction


function [nn, mean, std] = nn_do_train(data, cols)
  # generate a random set of N tids

  train_tids = gen_tids(200);
  # train_tids = [1:300];
  # train_tids = [1:100];
  # train_set = nn_get_training_set_deriv_only(data, cols, train_tids);
  train_set = nn_get_training_set_deriv(data, cols, train_tids);
  # train_set = nn_get_training_set(data, cols, train_tids);
  [nn, mean, std] = nn_train(train_set);
  return ;

  # iteration count
  iter_count = 10;

  for i = 1:iter_count
    printf("----- GENERATION %d\n", i);

    # train the net
    train_set = nn_get_training_set(data, cols, train_tids);
    [nn, mean, std] = nn_train(train_set);

    # generate test tids
    test_tids = gen_tids(20);

    # get the scores
    scores = nn_do_score(data, nn, mean, std, cols, test_tids);

    # add bad scoring tid to training set or done
    bad_scores = find(scores <= 0.6);

    if length(bad_scores) == 0 return ; end # we are done

    scores(bad_scores)

    train_tids = [train_tids test_tids(bad_scores)];
  end

endfunction


function nn_save(nn, filename)
  # to save the nn, must be called like this: save(filename, "nn");
  # "nn" must be the stringified variable name
  # then the nn can be load with nn = load(filename);
endfunction


function train_set = nn_get_training_set_deriv(data, cols, tids)
  # row format: [ alert, sample0, sample1, deriv0, deriv1 ]

  col_count = length(cols);

  first_sampl_col = 2;
  last_sampl_col = first_sampl_col + col_count - 1;
  first_deriv_col = last_sampl_col + 1;
  last_deriv_col = first_deriv_col + col_count - 1;

  train_set = zeros(1, last_deriv_col);

  # train_row indexes the current training set
  train_row = 1;

  # filter the original rows
  for tid = tids
    rows = find(data(:,1)==tid);
    rows = rows'; # transposed

    row_count = length(rows);
    if row_count <= 1 continue; end

    # perchunk trainset expansion
    train_set(train_row:train_row + row_count - 1,:) = zeros();

    # first row handled separately

    row = rows(1);
    train_set(train_row, 1) = data(row, 3);
    train_set(train_row, first_sampl_col:last_sampl_col) = data(row, cols);
    train_set(train_row, first_deriv_col:last_deriv_col) = zeros(1, col_count);
    train_row = train_row + 1;
    rows(1) = [];

    # foreach row, get data and signal derivative
    for row = rows
      # assign signal samples
      train_set(train_row, 1) = data(row, 3);
      train_set(train_row, first_sampl_col:last_sampl_col) = data(row, cols);
      # compute derivatives
      train_set(train_row, first_deriv_col:last_deriv_col) = data(row, cols) - data(row - 1, cols);
      # next training set row
      train_row = train_row + 1;
    end
  end
endfunction


function train_set = nn_get_training_set_deriv_only(data, cols, tids)
  train_set = nn_get_training_set_deriv(data, cols, tids);
  train_set(:, cols) = [];
endfunction


function [test_set, res_set] = nn_get_submission_set_deriv(data, cols)
  # get the list of unique tids
  tids = unique(data(:,1));
  tids = tids';
  test_set = nn_get_training_set_deriv(data, cols, tids);
  res_set = test_set(:,1);
  test_set(:,1) = [];
endfunction


function [test_set, res_set] = nn_get_submission_set_deriv_only(data, cols)
  [test_set, res_set] = nn_get_submission_set_deriv(data, cols);
  test_set(:, cols) = [];
endfunction


function [test_set, res_set] = nn_get_testing_set_deriv(data, cols, tids)
  test_set = nn_get_training_set_deriv(data, cols, tids);
  res_set = test_set(:,1);
  test_set(:,1) = [];
endfunction


function [test_set, res_set] = nn_get_testing_set_deriv_only(data, cols, tids)
  [test_set, res_set] = nn_get_testing_set_deriv(data, cols, tids);
  test_set(:, cols) = [];
endfunction


function nn_diff(data)
  deriv = diff(data)
endfunction

function val_stats = nn_stats_value(data, value)
  val_rows = find(data(:,3) == value)';
  val_data = data(val_rows,:);

  # sid['median', 'mean', 'dev', 'min', 'max', 'mode', 'freq']
  val_stats = zeros(33, 7);
  for sid = 4:33
    val_stats(sid,1) = median(val_data(:, sid));
    val_stats(sid,2) = mean(val_data(:, sid));
    val_stats(sid,3) = std(val_data(:, sid));
    val_stats(sid,4) = min(val_data(:, sid));
    val_stats(sid,5) = max(val_data(:, sid));
    [val_stats(sid,6), val_stats(sid,7), dummy] = mode(val_data(:, sid));
  end
endfunction

function nn_print_stats(zero_stats, one_stats)
  for sid = 4:33
    printf("[%d]\n", sid);
    for col = 1:7 printf(" %.3f", zero_stats(sid,col)); end
    printf("\n");
    for col = 1:7 printf(" %.3f", one_stats(sid,col)); end
    printf("\n");
    printf("\n");
  end
endfunction

function nn_stats(data)
  zero_stats = nn_stats_value(data, 0);
  one_stats = nn_stats_value(data, 1);
  nn_print_stats(zero_stats, one_stats);
endfunction