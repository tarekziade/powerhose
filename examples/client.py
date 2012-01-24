from powerhose import PowerHose
from job_pb2 import Job
import cProfile
import tempfile
import pstats
import time
import sys

def profile(sort='cumulative', lines=50, strip_dirs=False):

    def outer(fun):
        def inner(*args, **kwargs):
            file = tempfile.NamedTemporaryFile()
            prof = cProfile.Profile()
            try:
                ret = prof.runcall(fun, *args, **kwargs)
            except:
                file.close()
                raise

            prof.dump_stats(file.name)
            stats = pstats.Stats(file.name)
            if strip_dirs:
                stats.strip_dirs()
            if isinstance(sort, (tuple, list)):
                stats.sort_stats(*sort)
            else:
                stats.sort_stats(sort)
            stats.print_stats(lines)

            file.close()
            return ret
        return inner

    # in case this is defined as "@profile" instead of "@profile()"
    if hasattr(sort, '__call__'):
        fun = sort
        sort = 'cumulative'
        outer = outer(fun)
    return outer


def showtime(func):
    def _showtime(*args, **kw):
        start = time.time()
        res = func(*args, **kw)
        print "Total time is %.2f" % (time.time() - start)
        return res
    return _showtime


@showtime
@profile
def work():
    with PowerHose() as ph:

        for x in range(1000):
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


if __name__ == "__main__":

    work()
    sys.exit()
