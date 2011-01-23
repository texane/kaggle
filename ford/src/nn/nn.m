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
  ti = ds(2:size(ds)(1), 4:ncols);
  ti = ti';
endfunction

function [ti, to] = nn_get_training_sets(ds)
  # ti, to the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  rows = find(ds(2:size(ds)(1),2)<200);

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
  size(in_foo)
  size(to)
  [nn] = train(net, in_foo, to, [], [], VV);
  mean = in_mean;
  std = in_std;
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
  out = nn_eval(nn, mean, std, ds);
  isalert = ds(2:size(ds)(1),3);
  score = 0;
  for i = 1:size(out)(1)
    if out(i) == isalert(i) score = score + 1; end
  end
  score = score / size(ds)(1);
endfunction

function nn_submit(td, res)
  # td the test set
  # res the results
  file = fopen("/tmp/fu.csv", "wt");
  fwrite(file, "TrialID,ObsNum,Prediction\n");
  for i = 2:size(td)(1)
    fprintf(file, "%d,%d,%d\n", td(i,1), td(i,2), res(i-1));
  end
  fclose(file);
endfunction
