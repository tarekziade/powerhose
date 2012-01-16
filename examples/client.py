from powerhose.sender import Sender
from job_pb2 import Job


if __name__ == "__main__":

    # Start the ventilator!
    ventilator = Sender()

    try:
        # sending some jobs
        for i in xrange(1, 10, 4):
            job = Job()
            job.value = i
            result = ventilator.execute('square', job.SerializeToString())
            res = job.FromString(result)
            print '%d * %d = %d' % (i, i, res.value)

    finally:
        ventilator.stop()
