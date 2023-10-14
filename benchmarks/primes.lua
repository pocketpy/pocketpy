local UPPER_BOUND = 5000000
local PREFIX = 32338

local Node = {}
function Node:new()
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    obj.children = {}
    obj.terminal = false
    return obj
end

local Sieve = {}
function Sieve:new(limit)
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    obj.limit = limit
    obj.prime = {}
    for i = 0, limit do
        obj.prime[i] = false
    end
    return obj
end

function Sieve:to_list()
    local result = {2, 3}
    for p = 5, self.limit do
        if self.prime[p] then
            table.insert(result, p)
        end
    end
    return result
end

function Sieve:omit_squares()
    local r = 5
    while r * r < self.limit do
        if self.prime[r] then
            local i = r * r
            while i < self.limit do
                self.prime[i] = false
                i = i + r * r
            end
        end
        r = r + 1
    end
    return self
end

function Sieve:step1(x, y)
    local n = (4 * x * x) + (y * y)
    if n <= self.limit and (n % 12 == 1 or n % 12 == 5) then
        self.prime[n] = not self.prime[n]
    end
end

function Sieve:step2(x, y)
    local n = (3 * x * x) + (y * y)
    if n <= self.limit and n % 12 == 7 then
        self.prime[n] = not self.prime[n]
    end
end

function Sieve:step3(x, y)
    local n = (3 * x * x) - (y * y)
    if x > y and n <= self.limit and n % 12 == 11 then
        self.prime[n] = not self.prime[n]
    end
end

function Sieve:loop_y(x)
    local y = 1
    while y * y < self.limit do
        self:step1(x, y)
        self:step2(x, y)
        self:step3(x, y)
        y = y + 1
    end
end

function Sieve:loop_x()
    local x = 1
    while x * x < self.limit do
        self:loop_y(x)
        x = x + 1
    end
end

function Sieve:calc()
    self:loop_x()
    return self:omit_squares()
end

local function generate_trie(l)
    local root = Node:new()
    for _, el in ipairs(l) do
        local head = root
        -- attempt to call a nil value (method 'split')
        -- how to fix? use string.split
        el = tostring(el)
        for i=1, #el do
            local ch = el:sub(i, i)
            if not head.children[ch] then
                head.children[ch] = Node:new()
            end
            head = head.children[ch]
        end
        head.terminal = true
    end
    return root
end

local function find(upper_bound, prefix_)
    local primes = Sieve:new(upper_bound):calc()
    local str_prefix = tostring(prefix_)
    local head = generate_trie(primes:to_list())
    for i=1, #str_prefix do
        local ch = str_prefix:sub(i, i)
        head = head.children[ch]
        if head == nil then
            return nil
        end
    end

    local queue, result = {{head, str_prefix}}, {}
    while #queue > 0 do
        local tuple = table.remove(queue)
        local top, prefix = tuple[1], tuple[2]
        if top.terminal then
            table.insert(result, tonumber(prefix))
        end
        for ch, v in pairs(top.children) do
            table.insert(queue, 1, {v, prefix .. ch})
        end
    end

    table.sort(result)
    return result
end

local function verify()
    local left = {2, 23, 29}
    local right = find(100, 2)
    if #left ~= #right then
        print("length not equal")
        os.exit(1)
    end
    for i, v in ipairs(left) do
        if v ~= right[i] then
            print(string.format("%s != %s", v, right[i]))
            os.exit(1)
        end
    end
end

verify()
local results = find(UPPER_BOUND, PREFIX)
local expected = {323381, 323383, 3233803, 3233809, 3233851, 3233863, 3233873, 3233887, 3233897}

for i, v in ipairs(results) do
    if v ~= expected[i] then
        print(string.format("%s != %s", v, expected[i]))
        os.exit(1)
    end
end

