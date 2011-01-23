function [ti, to] = nn_get_eval_sets(ds, count)
  # ti, to the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  rows = find(ds(:,2)==2); # second trial

  if count > size(rows)(1) count = size(rows)(1) end
  rows = rows(1:count,:);

  # transposed
  ti = ds(rows, 4:ncols);
  to = ds(rows, 3);
  ti = ti';
  to = to';
endfunction

function [ti, to] = nn_get_training_sets(ds)
  # ti, to the training set input, output values
  ncols = size(ds)(2);
  nelems = ncols - 3;
  rows = find(ds(:,2)==0);

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
  [ti, to] = nn_get_eval_sets(ds, 100);
  [mTestInputN] = trastd(ti, mean, std);
  [simout] = sim(nn, mTestInputN);
  rows = size(to)(2);
  outs = zeros(rows, 3);
  for i = 1:rows
    value = simout(1,i);
    outs(i,1) = value;
    if value < 0.5 value = 0.0;
    else value = 1.0; end
    outs(i,2) = value;
    outs(i,3) = to(1,i);
  end
  score = length(find(outs(:,2) == outs(:,3))) / size(outs)(1);
  printf("score == %f\n", score);
endfunction
