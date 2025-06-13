from dataclasses import dataclass, asdict

@dataclass
class A:
    x: int
    y: str = '123'

assert repr(A(1)) == "A(x=1, y='123')"
assert repr(A(x=3)) == "A(x=3, y='123')"
assert repr(A(1, '555')) == "A(x=1, y='555')"
assert repr(A(x=7, y='555')) == "A(x=7, y='555')"

assert asdict(A(1, '555')) == {'x': 1, 'y': '555'}

assert A(1, 'N') == A(1, 'N')
assert A(1, 'N') != A(1, 'M')

#################

@dataclass
class Base:
  i: int
  j: int

class Derived(Base):
  k: str = 'default'

  def sum(self):
    return self.i + self.j

d = Derived(1, 2)

assert d.i == 1
assert d.j == 2
assert d.k == 'default'
assert d.sum() == 3


@dataclass
class PrimaryForceConfig:
    # 风场图
    planetary_wind: 'str'
    local_wind: int
    # 地壳运动（含地震带/地形生成）
    geothermal_activity: int
    # 太阳辐射标量场
    solar_radiation: int
    # 水汽场
    planetary_humidity: int

config = PrimaryForceConfig(
   planetary_wind = 'default',
   local_wind = 1,
   geothermal_activity = 2,
   solar_radiation = 3,
   planetary_humidity = 4
)

assert config.planetary_wind == 'default'