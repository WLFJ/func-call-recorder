'''
这里放一些全局配置
'''

from pathlib import Path

class Config:
    injector_path: Path = Path("./lib/libInjector.so")

    def __init__(self):
        '''
        将配置当中的相对位置更换为绝对位置
        '''
        # pyinjector包的位置
        import os
        package_base_path = Path(os.path.dirname(os.path.abspath(__file__)))

        self.injector_path = package_base_path / self.injector_path

