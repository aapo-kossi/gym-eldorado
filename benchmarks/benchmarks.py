from asv_runner.benchmarks.mark import SkipNotImplemented
import eldorado_py

N_STEPS = 10_000

class TimeEnvs:
    timeout = 300
    params = (
        [1,2,3,4,5,6,7,8,16,32,64,128,256],
        [12345],
        [1,2,3,4,5],
        ["sequential", "async", "sync"],
    )

    def setup(self, n, seed, threads, mode):
        if ((mode == "sequential") and (threads > 1)): raise NotImplementedError()
        self.threaded = mode != "sequential"
        synced = mode == "sync"
        env_cls = eldorado_py.get_vec_env(n)
        sampler_cls = eldorado_py.get_vec_sampler(n)
        envs = env_cls()
        envs.reset(seed, 4, 3, eldorado_py.Difficulty.EASY, 100000, False)
        self.envs = envs
        samplers = sampler_cls(seed)
        self.samplers = samplers
        self.actions = samplers.get_actions()
        self.am = envs.selected_action_masks
        if self.threaded:
            runner = eldorado_py.get_runner(n)(envs, samplers, threads)
            self.sample = runner.sample
            if synced:
                self.step_func = runner.step_sync
                self.end_fun = lambda: None
                self.sync_fun = lambda: runner.sync()
            else:
                self.step_func = runner.step
                self.end_fun = lambda: runner.sync()
                self.sync_fun = lambda: None
        else:
            self.step_func = lambda: envs.step(self.actions)
            self.sample = lambda: samplers.sample(self.am)
            self.end_fun = lambda: None
            self.sync_fun = self.end_fun
        self.reset = envs.reset

    def time_run(self, *_):
        for _ in range(N_STEPS):
            self.sample()
            self.step_func()
        self.end_fun()

    def time_sample(self, *_):
        for _ in range(N_STEPS):
            self.sample()
            self.sync_fun()
        self.end_fun()

    def time_reset(self, *_):
        if self.threaded:
            raise SkipNotImplemented
        for _ in range(N_STEPS//10):
            self.reset()

    def peakmem_runner(self, *_):
        self.sample()
        self.step_func()
        self.sync_fun()
        self.end_fun()

