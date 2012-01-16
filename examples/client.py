from powerhose import PowerHose
from job_pb2 import Job


if __name__ == "__main__":

    with PowerHose() as ph:
        # sending some jobs
        for i in xrange(1, 10, 4):
            job = Job()
            job.value = i
            status, result = ph.execute('square', job.SerializeToString())
            if status != 'OK':
                print 'The job has failed'
                print result
            else:
                res = job.FromString(result)
                print '%d * %d = %d' % (i, i, res.value)

    with PowerHose() as ph:
        print ph.execute('nothing', 'xxx')

