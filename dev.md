# dev

## Process
```FileSystem```(30%)
- ```Save```
  - ```Config```
- ```Language```
  - ```Translator```(30%)
- ```Texture```
- ```KeyBinding```(20%)
- ```World```
  - ```Entity```(10%)
- ```UI```(30%)
  - Maybe we need a ```ScrollList``` widget

## Note
### Structure
- 这个栏目写一些游戏的代码结构，防止后面忘了
- 关于**CMakeList.txt**和**includes.h**
  - 这两个文件非常重要，是游戏的编译和链接的核心。
  - <font color="#ee6666">头文件的include顺序管理就在这里</font>
  - **CMakeList.txt**是cmake的配置文件，包含了所有的源文件。
  - **includes.h**是游戏的头文件，包含了所有的头文件（目前，DirectX相关内容并没有包含在内）。
  - 上面的两个文件，包含了游戏中所有头文件的包含顺序。所有头文件在编写后必须插入**includes.h**中；如果有头文件，也必须插入**CMakeList.txt**中，如果没有，则在对应的位置插入一行注释。
  - 头文件代码中的`#include`顺序必须严格按照**includes.h**的顺序引用，不能乱序。
  - 在源文件代码中，可以包含对应头文件后面的文件，但是所有的包含顺序也必须与**includes.h**相同。
- `class GarbageCollector`是垃圾回收管理器。
  - 垃圾回收管理器是保证游戏内存安全的最重要的一个类。
  - ***任何有直接操作`operator new`或者`operator delete`的代码都应当呗封装起来，而不是直接使用这两个操作符。***
  - 垃圾回收管理器会在游戏运行时自动删除所有不再使用的对象。
  - 垃圾回收管理器保证线程安全，保证渲染线程和游戏线程的同步后再删除对象。
  - 如果有需要手动删除的对象，***必须考虑***该对象是否会同时被多个线程使用。如果是，那么建议考虑使用垃圾回收管理器（或者特化`class Garbage<TypeName>`）
- `class IRenderer`是渲染接口类，目前只实现了`class GdiRenderer`类。对**D3D12**的支持以后再说，说不定不考虑了
- `class Game`是游戏类，管理几乎所有的游戏资源。唯一实例定义在**game.h**中
  - 游戏管理器
    - `class WorldManager`管理所有的世界。所有的世界必须提交到管理器中以后才能被执行、更新，才会参与运算。
      - 世界被移除时（`WorldManager::removeWorld`），会自动删除其中的所有方块、实体等，并将实体从`EntityManager`中移除。
      - 注意，当一个方块从世界中移除时，不会被删除，这是为了以后的方块转移到其他世界等情况考虑的。如果需要删除，必须手动删除。`Block::onRemove`
    - `class EntityManager`管理所有的实体。所有的实体必须提交到管理器中统一管理，防止内存泄露。
      - 一般来说，实体的更新是由包含它的世界完成的。如果要实现多个世界之间的交叉，需要额外的机制自行实现。
  - 游戏资源
    - `class Block`是所有方块类继承的方块基类。
      - 当一个方块从世界中删除时，考虑到可能是移动，方块不会被自动delete（或者说，调用方块的`Block::onRemove`）。***注意：调用`World::removeBlock`或者`World::removeBlockAt`后，程序必须手动调用`Block::onRemove`***
      - 当一个方块需要**彻底删除**时，delete逻辑***必须***写在`onRemove`中。`Block::onRemove`虚函数默认实现了提交给gc的删除逻辑。
    - `class Entity`是所有实体类继承的实体基类。
      - 彻底删除一个实体的办法就是，从`class EntityManager`里删除。但是，删除之前，必须保证该实体已经从对应的世界中删除。***注意：从EntityManager中移除后，不需要调用`Entity::onRemove`***
      - 当一个实体需要**彻底删除**时，delete逻辑***必须***写在`onRemove`中。`Entity::onRemove`虚函数默认实现了提交给gc的删除逻辑。
