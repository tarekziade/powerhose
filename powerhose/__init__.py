from powerhose.workers import create_pool
from powerhose.sender import Sender


if __name__ == "__main__":

    def square(num, **kwargs):
        print 'job done'
        return num * num

    funcs = {'square': square}

    # Create a pool of workers to distribute work to
    create_pool(10, funcs)

    # Start the ventilator!
    ventilator = Sender()

    # sending a job
    for i in xrange(1, 10, 4):
        ventilator.execute({'num': i, 'func': 'square'})

    ventilator.stop()
