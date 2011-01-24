function data = nn_readcsv(filename)
  # remove the very first list
  data = csvread(filename);
  data(1,:) = [];
endfunction

function ts = nn_get_testing_set(ds)
  # get a subset for testing
  ts = find(ds(:,2)==2);
  ts = resize(ts, 100, size(ts)(2));
endfunction

function ti = nn_get_eval_set(ds)
  # ti the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  # transposed
  ti = ds(1:size(ds)(1), 4:ncols);
  ti = ti';
endfunction

function [ti, to] = nn_get_training_sets(ds)
  # ti, to the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  rows = find(ds(1:size(ds)(1),2)<10);

  # transposed
  ti = ds(rows, 4:ncols);
  to = ds(rows, 3);
  ti = ti';
  to = to';
endfunction

function [nn, mean, std] = nn_train(ds)
  # ds the data set from csv
  # ti the training input values
  # to the training output values

  # get the training sets
  [ti, to] = nn_get_training_sets(ds);

  # standardize the inputs
  [in_foo, in_mean, in_std] = prestd(ti);

  # hidden and output layers neurons count
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

function outs = nn_sim(nn, mean, std, ds)
  # output the proba
  # nn the neural network from nn_train
  # simin the simulation input
  ti = nn_get_eval_set(ds);
  [mTestInputN] = trastd(ti, mean, std);
  [simout] = sim(nn, mTestInputN);
  rows = size(ti)(2);
  outs = zeros(rows, 1);
  for i = 1:rows
    outs(i) = simout(1, i);
  end
endfunction

function outs = nn_eval(nn, mean, std, ds)
  # nn the neural network from nn_train
  # simin the simulation input
  ti = nn_get_eval_set(ds);
  [mTestInputN] = trastd(ti, mean, std);
  [simout] = sim(nn, mTestInputN);
  rows = size(ti)(2);
  outs = zeros(rows, 1);
  for i = 1:rows
    value = simout(1,i);
    if value < 0.5 value = 0.0;
    else value = 1.0; end
    outs(i) = value;
  end
endfunction

function score = nn_score(nn, mean, std, ds)
  # nn the trained nn
  ts = nn_get_testing_set(ds);
  out = nn_sim(nn, mean, std, ds);
  isalert = ds(1:size(ds)(1),3);
  score = 0;
  for i = 1:size(out)(1)
    value = 1;
    if out(i) < 0.5 value = 0; end
    if value == isalert(i) score = score + 1; end
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

function nn_choose_net(ds)
  # use a genetic algorithm to find the sample to use

  # current sample weights. start with equiprobability.
  sample_weights = zeros(1, 34);

  # number of generation
  gen_count = 20;

  # population count per generation
  pop_per_gen = 20;

  # per population sample count
  sample_per_pop = 10;

  # generate the training sample set list
  for each(gen_count)
    # initialize the resulting weights
    prev_weights = sample_weights;

    for each(pop_per_gen)
      # generate a list of random sample with more weight on strongest
      for each(sample_per_pop)
	samples[] = rand();
      end

      # get the training data set and train the nn
      td = subset(data, samples);
      [nn, mean, std] = nn_train(td);

      # get the resulting scores
      scores(i, 1) = nn_score(nn, mean, std, data);
      scores(i, 2) = samples;
    end # foreach(population)

    # sort the scores
    [sort_val, sort_ind] = sort(scores, 'ascend');

    # update sample weights
    for i = 1:size(sort_ind)(1)
      for j = 1:sample_per_pop # samples for this score
        # use the sorted pos i as a weight
	sample_weights(scores(sort_ind(i), j)) += i;
    end

  end # foreach(generation)

  # print the resulting weights
  [sort_val, sort_ind] = sort(sample_weights);
  for i = 1:size(sort_val)(1)
    printf("%f %f\n", sort_ind(i), sort_val(i));
  end

endfunction
