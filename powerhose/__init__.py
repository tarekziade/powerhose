from powerhose.sender import Sender
from powerhose.job_pb2 import Job


if __name__ == "__main__":

    # Start the ventilator!
    ventilator = Sender()

    try:
        # sending some jobs
        for i in xrange(1, 10, 4):
            job = Job()
            job.func = 'square'
            job.param = i
            job.id = 1
            result = ventilator.execute('square', job.SerializeToString())
            res = Job()
            res = res.FromString(result)
            print '%d * %d = %d' % (i, i, res.param)

    finally:
        ventilator.stop()
