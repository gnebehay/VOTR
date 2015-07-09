import cvbb
import json
import os
import subprocess
import tempfile
import time

def load_trackers():

    #Load root dir entry from file
    tracker_file = os.path.join(os.path.expanduser("~"), '.cv', 'trackers')

    with open(tracker_file) as f:
        trackers_json = json.load(f)
        tracker_names = trackers_json.keys()

        trackers = {}

        for t in tracker_names:
            tracker = Tracker()
            tracker.name = t

            track_obj = trackers_json[t]
            tracker.command = track_obj['command']
            if 'blacklist' in track_obj:
                tracker.blacklist = track_obj['blacklist']
            else:
                tracker.blacklist = []
            if 'protocol' in track_obj:
                tracker.protocol = track_obj['protocol']

            trackers[tracker.name] = tracker;

        return trackers

def load_selection(selection_file):

    with open(selection_file) as f:
        selection = json.load(f)

        trackers = load_trackers()

        selected_trackers = []

        for t in selection['selection']:

            name = t.keys()[0]

            #find tracker
            tracker = trackers[name]

            #Rename tracker
            tracker.name = t[name]

            selected_trackers.append(tracker)

        return selected_trackers

class Tracker:

    name = None
    command = None
    protocol = 'BB'

    def run(self, sequence, additional_args = ''):

        #Create working directory
        working_dir = tempfile.mkdtemp()

        print 'working dir is', working_dir

        #Save list of images to working directory
        image_file = os.path.join(working_dir, 'images.txt')
        with open(image_file, 'w') as f:
            f.write('\n'.join(sequence.im_list))

        #Save init file to working directory
        init_file = os.path.join(working_dir, 'region.txt')


        if self.protocol == 'BB':
            init_region = sequence.gt[[0],:]
        elif self.protocol == 'POLY':
            init_region = cvbb.bb2poly(sequence.gt[[0],:])
        else:
            raise Exception('Unknown input protocol')

        cvbb.write(init_file, init_region)

        #Remember current directory
        current_dir = os.getcwd()

        #Change to working directory
        os.chdir(working_dir)

        if additional_args == '':
            cmd = self.command
        else:
            cmd = self.command + ' ' + additional_args

        #Run the tracker
        tic = time.time()
        print "Running tracker " +  self.name +  " on sequence " + sequence.name \
            + " from dataset " + sequence.dataset + " ( " + str(sequence.num_frames)  + " frames) using command "  + cmd
        subprocess.call(cmd, shell=True)
        toc = time.time()
        elapsed_time = toc - tic

        #Change directory back to old directory
        os.chdir(current_dir)

        #Create path to output file
        output_file = os.path.join(working_dir, 'output.txt')

        if not os.path.exists(output_file):
            raise Exception('No output file was generated')

        #Read output file
        results = cvbb.read(output_file)

        if sequence.gt is not None:
            if not results.shape[0] == sequence.gt.shape[0]:
                raise Exception('Number of output frames does not match number of GT frames')

        return results, elapsed_time

if __name__ == '__main__':
    import sys
    load_trackers(sys.argv[1])

