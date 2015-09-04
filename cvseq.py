import json
import numpy as np
import os
import cv2

def load_sequences(top_dir = None):

    if top_dir is None:
        #Load root dir entry from file
        seq_file = os.path.join(os.path.expanduser("~"), '.cv', 'dataset')

        with open(seq_file) as f:
            seq_json = json.load(f)
            top_dir = seq_json['root_dir']

    print('Listing sequences in directory ' + top_dir)

    if os.path.exists(os.path.join(top_dir,'datasets')):
        datasets = True
    else:
        datasets = False

    sequences = {}

    if datasets == True:
        #Top level directories are dataset names
        dirs = [d for d in os.listdir(top_dir) if os.path.isdir(os.path.join(top_dir, d))]
    else:
        dirs = ['.']

    for dataset in dirs:
        dataset_dir = os.path.join(top_dir, dataset)
        seq_dirs = [d for d in os.listdir(dataset_dir) if os.path.isdir(os.path.join(top_dir, dataset, d))]
        for seq_name in seq_dirs:
            directory = os.path.join(dataset_dir, seq_name)
            seq = Sequence()
            seq.dataset = dataset
            seq.directory = directory
            seq.name = seq_name
            if datasets == True:
                seq.identifier = dataset + '.' + seq_name
            else:
                seq.identifier = seq_name
            sequences[seq.identifier] = seq
            #Find groundtruth file
            gt_file = os.path.join(directory, 'groundtruth.txt')
            if os.path.exists(gt_file):
                #Read groundtruth
                seq.gt = np.genfromtxt(gt_file, delimiter=',')

            else:
                seq.gt = None
                print('Warning: sequence ' + seq.identifier + " doesn't have a groundtruth file.")

            seq.load()

    return sequences

def load_selection(selection_file):

    with open(selection_file) as f:

        selected_sequences = [l.strip() for l in f.readlines()]

        seqs = [seq for seq in load_sequences().values() if seq.identifier in selected_sequences]

    return seqs

class Sequence:

    def __init__(self):
        self.dataset = None
        self.gt = None
        self.im_list = None
        self.name = None
        self.num_frames = None
        self.directory = None
        self.identifier = None

    def load(self):

        if self.im_list is not None:
            return

        print('Loading sequence ' + self.identifier)

        first_file = os.path.join(self.directory, '00000001.jpg')

        #Test for file extension
        if os.path.exists(first_file):
            file_ext = '.jpg'
        else:
            file_ext = '.png'
            first_file = os.path.join(self.directory, '00000001.png')

        im = cv2.imread(first_file)

        self.shape = im.shape

        #Create list of images
        im_list = []

        MAX_IM = 10000000

        for i in xrange(1,MAX_IM):
            im_file = '{0:08d}{1}'.format(i,file_ext)
            im_path = os.path.join(self.directory, im_file)
            if os.path.exists(im_path):
                im_list.append(im_path)
            else:
                break

        self.im_list = im_list

        #Compute number of frames
        self.num_frames = len(im_list)

        if self.gt is not None:
            gt_len = self.gt.shape[0]

            if gt_len != self.num_frames:
                raise Exception('Number of entries in groundtruth file differs from number of frames in sequence ' + self.identifier + '.')
