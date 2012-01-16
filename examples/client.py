from powerhose.sender import PowerHose
from job_pb2 import Job


if __name__ == "__main__":

    # Start the ventilator!
    with PowerHose() as ph:
        # sending some jobs
        for i in xrange(1, 10, 4):
            job = Job()
            job.value = i
            result = ph.execute('square', job.SerializeToString())
            res = job.FromString(result)
            print '%d * %d = %d' % (i, i, res.value)
