#!/usr/bin/env python

import itertools
import numpy as np
import os

import cvbb
import cvtrack
import cvseq

from scipy.stats import nanmean

def overlap(output,gt):

    #Calculate overlap
    overlap = cvbb.overlap(output, gt)
    gt_defined = ~np.any(np.isnan(gt), axis=1)
    output_defined = ~np.any(np.isnan(output), axis=1)

    return overlap, output_defined, gt_defined

def compute_average(overlap, output_defined, gt_defined):

    #Set all per frame measures to 0 where gt is defined and output is not defined
    only_gt_defined = ~output_defined & gt_defined
    overlap[only_gt_defined] = 0

    #Now compute average only on those sequences
    avg = np.nanmean(overlap)

    return avg

def compute_recall_precision(overlap, output_defined, gt_defined, theta):

    #Set all per frame measures to 0 where gt is defined and output is not defined
    only_gt_defined = ~output_defined & gt_defined
    overlap[only_gt_defined] = 0 #

    #Calculate TP, FP, FN based on threshold
    TP = overlap > theta
    FP = (overlap < theta) | (~gt_defined & output_defined)
    FN = (overlap < theta) | (gt_defined & ~output_defined)

    #Compute precision
    precision = np.sum(TP, dtype=np.float) / (sum(TP) + sum(FP))

    #Compute recall
    recall = np.sum(TP, dtype=np.float) / (sum(TP) + sum(FN))

    return recall, precision

def compute_success_plot(X):
    X_rounded = np.floor(X*100)/100
    U = np.sort(X_rounded)
#    if U[0] != 0:
#        U = np.append(0,U)
    if U[-1] != 1:
        U = np.append(U,1)
    N = len(X)
    P = []
    lastu = 0
    A = N
    for u in U:
        if not u == lastu:
            P.append((u,A*1.0/N))
        A = A - 1
        lastu = u

    P = [(0,P[0][1])] + P

    return np.array(P)

def cvpr2013(name, trackers, seqs, outcomes, output_dir):

    print 'performing evaluation according to Wu'

    m = len(trackers)
    n = len(seqs)

    for i in xrange(m):
        print 'evaluating ' + trackers[i].name

        non_missing = np.array([o is not None for o in outcomes[i,:]])

        num_missing = n - len(non_missing)

        if num_missing > 0:
            print 'Warning: ' + str(num_missing) + ' sequences have no result'

        seqs_non_missing = itertools.compress(seqs, non_missing)

        #Lump everything together
        gt = np.vstack([seq.gt for seq in seqs_non_missing])

        outcome = np.vstack(outcomes[i,non_missing])

        O, output_defined, gt_defined = overlap(outcome, gt)

        #Consider only those frames where gt is defined
        O = O[gt_defined]

        #Set overlap to 0 if algorithmic result is undefined
        O[np.isnan(O)] = 0

        success_plot = compute_success_plot(O)

        np.savetxt(os.path.join(output_dir, name + '_success_plot_cvpr_' + trackers[i].name + '.txt'), success_plot, delimiter=',')

def wacv2014(name, trackers, seqs, outcomes, output_dir):

    print 'performing evaluation according to wacv paper'

    m = len(trackers)
    n = len(seqs)

    recalls = np.empty(outcomes.shape)

    for theta in [0.25, 0.5, 0.75]:
        recalls[:] = np.nan
        for i in xrange(m):
            for j in xrange(n):
                if outcomes[i,j] is None:
                    print 'Warning: Skipping sequence ' + seqs[j].name
                    continue
                gt = seqs[j].gt
                O, output_defined, gt_defined = overlap(outcomes[i,j], gt)
                recall, _ = compute_recall_precision(O, output_defined, gt_defined, theta)
                recalls[i,j] = recall

            #Write successplot
            success_plot = compute_success_plot(recalls[i,~np.isnan(recalls[i,:])])
            #Add 0,1
            success_plot = np.vstack(([0,1], success_plot))
            np.savetxt(os.path.join(output_dir, name + '_success_plot_wacv_' + str(theta) + '_' + trackers[i].name + '.txt'), success_plot, delimiter=',')

def table(name, trackers, seqs, outcomes, output_dir):

    print 'Creating table'

    n = len(trackers)
    m = len(seqs)

    recalls = np.empty(outcomes.T.shape)

    for theta in [0.5]:
        recalls[:] = np.nan
        for i in xrange(n):
            for j in xrange(m):
                if outcomes[i,j] is None:
                    print 'Warning: Skipping sequence ' + seqs[j].name
                    continue
                gt = seqs[j].gt
                O, output_defined, gt_defined = overlap(outcomes[i,j], gt)
                recall, _ = compute_recall_precision(O, output_defined, gt_defined, theta)
                recalls[j,i] = recall

        #Add average at the end
        avg = nanmean(recalls,axis=0)
        recalls = np.vstack((recalls,avg))

        with open(os.path.join(output_dir, name + '_recall.txt'), 'w') as f:
            f.write(','.join(['Sequence'] + [t.name for t in trackers]) + '\n')

            descs = [seq.identifier.replace('_', '\\_') for seq in seqs] + ['avg']
            for sequence, recall in zip(descs, recalls):
                f.write(sequence + ',')
                f.write(','.join(str(r) for r in recall) + '\n')

def compute_timing(name, trackers, seqs, timings, output_dir):

    m = len(trackers)

    with open(os.path.join(output_dir, name + '_fps.txt'),'w') as f:
        for i in xrange(m):
            non_missing = ~np.isnan(timings[i,:])

            seqs_non_missing = itertools.compress(seqs, non_missing)

            num_frames = np.sum([seq.num_frames for seq in seqs_non_missing])
            total_time = np.sum(timings[i,non_missing])
            fps = num_frames * 1.0 / total_time
            f.write(str(fps) + ', ' + trackers[i].name + '\n')

def evaluate(name, selection_file, sequence_file, outcome_dir, output_dir):

    trackers = cvtrack.load_selection(selection_file)
    seqs = cvseq.load_selection(sequence_file)

    output_dir = os.path.join(output_dir, 'plot')

    m = len(trackers)
    n = len(seqs)

    outcomes = np.empty((m, n),dtype=np.object)
    timings = np.empty((m, n))
    timings[:] = np.nan

    for i in xrange(m):

        tracker = trackers[i]

        tracker_outcome_dir = os.path.join(outcome_dir, tracker.name)

        if not os.path.exists(tracker_outcome_dir):
            os.mkdir(tracker_outcome_dir)

        for j in xrange(n):
            print('Sequence ' + str(j) + '/' + str(n))

            sequence = seqs[j]

            if sequence.identifier in tracker.blacklist:
                print(sequence.identifier + ' is in blacklist of ' + tracker.name)
                continue

            sequence_outcome_dir = os.path.join(tracker_outcome_dir, sequence.identifier)

            if not os.path.exists(sequence_outcome_dir):
                os.mkdir(sequence_outcome_dir)

            #Now: Run tracker on sequence or collect existing result
            outcome_file = os.path.join(sequence_outcome_dir, 'output.txt')
            timing_file = os.path.join(sequence_outcome_dir, 'timing.txt')

            if os.path.exists(outcome_file):
                print outcome_file, 'already exists, using cached version.'
                outcome = np.genfromtxt(outcome_file, delimiter=',')
                timing = np.genfromtxt(timing_file, delimiter=',')
            else:
                try:
                    [outcome, timing] = tracker.run(sequence)

                    #Cache output for next run
                    np.savetxt(outcome_file, outcome, delimiter=',')
                    with open(timing_file, 'w') as f:
                        f.write('{0}\n'.format(timing))
                except Exception:
                    print('Tracker ' + tracker.name + ' failed on sequence ' + sequence.identifier)
                    outcome = None
                    timing = None

            if outcome is not None:

                nPoints = outcome.shape[1] / 2.0
                if not nPoints == 2:
                    #Convert polygon to bounding box
                    outcome = cvbb.poly2bb(outcome)

            outcomes[i,j] = outcome
            timings[i,j] = timing

    #Then pass result to evaluation function
    cvpr2013(name, trackers, seqs, outcomes, output_dir)
    wacv2014(name, trackers, seqs, outcomes, output_dir)
    table(name, trackers, seqs, outcomes, output_dir)
    compute_timing(name, trackers, seqs, timings, output_dir)

    with open(os.path.join(output_dir, name + '_list.txt'), 'w') as f:
        for tracker in trackers:
            f.write(tracker.name + '\n')
