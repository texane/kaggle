function d = extract_signal(ds, tid, sid)
  # extract a signal from the dataset
  # ds the dataset
  # tid the trial id
  # sid the signal id
  # return d the signal data

  # get the signal range for tid
  rows = find(ds(:,1) == tid);

  # filter for the given sid
  d = ds(rows,sid);
endfunction

function n = count_tids(dataset)
  # return n the number of different tids
  # n = length(unique(dataset(:,1)));
  # cache value
  n = 500;
endfunction

function d = compute_dist(a, b)
  # compute distance between 2 signals
  # ds the dataset
  # return d the distance
  d = 0;
  [asize,foo] = length(a);
  [bsize,foo] = length(b);
  j = asize;
  if asize > bsize
    j = bsize;
  end
  for i = 1:j
    if a(i) != b(i)
      d = d + 1;
    end
  end
endfunction

function ds = compute_dists(dataset, tid, sid)
  # compute the distance between tid:sid
  # and all the other tid:sid
  a = extract_signal(dataset, tid, sid);
  tids = count_tids(dataset);
  ds = zeros(tids);
  for i = 1:tids
    if i != tid
      b = extract_signal(dataset, i, sid);
      ds(i) = compute_dist(a, b);
    end
  end

  # set to max zeros
  z = find( ds == 0 );
  for i = 1:length(z)
    ds(z(i)) = 20000;
  end
endfunction

function count = compute_crosses(ds, tid)
  # compute the alertness signal crossing feature
  count = 0;
  x = extract_signal(ds, tid, 3);
  if length(x) == 0 return end
  value = x(1);
  if length(x) == 1 return end
  for i = 2:length(x);
    if value != x(i)
      value = ~value;
      ++count;
    end
  end
endfunction

function groups = clusterize(ds, k)
  # compute the cluster based
  # k the culster count

  if nargin <= 1 k = 3; end

  # required
  source './km3.m';

  # build pair set (object, attr) for km2
  tids = count_tids(ds);
  # km2
  # pairs = zeros(tids, 2);
  # km3
  pairs = zeros(tids,1);
  for i = 1:tids
    # km2
    # pairs(i,:) = [i, compute_crosses(ds, i)];
    # km3
    pairs(i) = compute_crosses(ds, i);
  end

  # cluster groups are in last col
  # km2
  # groups = km3(pairs, k)
  # km3
  [cent, l, m] = km3(pairs, k);
  # m the membership matrix
  for i = 1:size(m)(1)
    res = find(m(i,:) == 1);
    if length(res) == 0 res = 0; end
    groups(i) = res;
  end
endfunction


function plot_group(ds, group, sid, max)

  if nargin < 4 max = length(group); end

  clf ;

  for i = 1:max
    x = extract_signal(ds, group(i), sid);
    subplot(max, 2, (i - 1) * 2 + 1);
    plot(x);

    x = extract_signal(ds, group(i), 4);
    subplot(max, 2, (i - 1) * 2 + 2);
    plot(x);
  end
endfunction

function group = get_initial_nonalert(ds)
  tids = count_tids(ds);
  j = 0;
  for i = 1:tids
    x = extract_signal(ds, i, 3);
    if length(x) && (x(1) == 0)
      j += 1;
      group(j) = i;
    end
  end
endfunction

function pair = do_initial(ds, sid)
  nas = get_initial_nonalert(ds);
  nas_avg = 0;
  for i = 1:length(nas)
    x = extract_signal(ds, nas(i), sid);
    if length(x) nas_avg += x(1); end
  end

  as = setxor(nas, [1:count_tids(ds)]);
  as_avg = 0;
  for i = 1:length(as)
    x = extract_signal(ds, as(i), sid);
    if length(x) as_avg += x(1); end
  end

  pair = [nas_avg / length(nas), as_avg / length(as)];
endfunction

function is_alert(ds, tid)
  na_avg = 35.582;
  a_avg = 34.012;

  x = extract_signal(ds, tid, 4);
  diff = [abs(na_avg - x(1)), abs(a_avg - x(1))];

  is_alert = 0;
  if diff(1) > diff(2) is_alert = 1; end

  # correct answer
  x = extract_signal(ds, tid, 3);
  if x(1) == is_alert
    printf("RIGHT: %d, %f, %f\n", is_alert, diff(1), diff(2));
  else
    printf("WRONG: %d, %f, %f\n", is_alert, diff(1), diff(2));
  end
endfunction

function score = score_alert(ds, sid)
  pair = do_initial(ds, sid);
  na_avg = pair(1);
  a_avg = pair(2);

  count = 0;
  score = 0;
  tids = count_tids(ds);
  for i = 1:tids
    x = extract_signal(ds, i, sid);
    if (length(x))
      ++count;
      diff = [abs(na_avg - x(1)), abs(a_avg - x(1))];
      is_alert = 0;
      if diff(1) > diff(2) is_alert = 1; end
      x = extract_signal(ds, i, 3);
      if x(1) == is_alert score = score+1; end
    end
  end
  score /= count;
endfunction
