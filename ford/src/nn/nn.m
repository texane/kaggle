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

function [test_set, res_set] = nn_get_testing_set(data, cols)
  # get a subset for testing
  # rows = find(data(:,1)<100);
  rows = find(data(:,2)==800);
  res_set = data(rows,3);
  test_set = data(rows,cols);
endfunction

function [test_set, res_set] = nn_get_submission_set(data, cols)
  # get a subset for testing
  res_set = data(:,3);
  test_set = data(:,cols);
endfunction

function ti = nn_get_eval_set(ds)
  # ti the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  # transposed
  ti = ds(1:size(ds)(1), 4:ncols);
  ti = ti';
endfunction

function train_set = nn_get_training_set(data, cols)
  # the first col contains the results
  rows = find(data(:,2)<100);
  train_set = data(rows, [3 cols]);
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
  nn_counts = [1, 1];
  # layers transfer function
  nn_funcs = {"tansig", "purelin"};

  # instanciate the network
  net = newff(min_max(in_foo), nn_counts, nn_funcs, "trainlm", "mse");

  # layer weights
  net.IW{1,1}(:) = 1.5;
  net.LW{2,1}(:) = 0.5;
  net.b{1,1}(:) = 1.5;
  net.b{2,1}(:) = 0.5;

  # disable plotting
  net.trainParam.show = NaN;

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


function nn_doit()
  # cols = [14 8 32 33 11 5 29 28 13 17 15 31 24 7 19 25 10 30 4 22];
  # cols = [4:21];
  cols = [4:33];

  data = nn_csvread('../../data/fordTrain.csv');

  train_set = nn_get_training_set(data, cols);
  [nn, mean, std] = nn_train(train_set);

  [test_set, res_set] = nn_get_testing_set(data, cols);
  score = nn_score(nn, mean, std, test_set, res_set);
  printf("score: %f\n", score);

  #submission_data = nn_csvread('../../data/fordTest.csv');
  #[submission_set, res_set] = nn_get_submission_set(submission_data, cols);
  #submission_res = nn_eval(nn, mean, std, submission_set);
  #nn_submit(submission_data, submission_res);
endfunction


function nn_diff(data)
  deriv = diff(data)
endfunction