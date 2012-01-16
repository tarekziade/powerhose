from powerhose.workers import create_pool
from powerhose.sender import Sender
from powerhose.job_pb2 import Job


if __name__ == "__main__":

    def square(num, **kwargs):
        print 'job done'
        return num * num

    funcs = {'square': square}

    # Create a pool of workers to distribute work to
    #create_pool(10, funcs)
    # Start the ventilator!
    ventilator = Sender()

    # sending a job
    for i in xrange(1, 10, 4):
        job = Job()
        job.func = 'square'
        job.param = i
        job.id = 1
        result = ventilator.execute('square', job.SerializeToString())
        res = Job()
        res = res.FromString(result)
        print '%d * %d = %d' % (i, i, res.param)

    ventilator.stop()
