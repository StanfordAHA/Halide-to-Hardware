#!/usr/bin/env python3
# Example usage: python compare_json.py file1.json file2.json

import json
import collections
import sys

def canonical_serialize(obj):
    if isinstance(obj, dict):
        items = []
        for key in sorted(obj.keys()):
            items.append('{}:{}'.format(repr(key), canonical_serialize(obj[key])))
        return '{' + ','.join(items) + '}'
    elif isinstance(obj, list):
        elements = [canonical_serialize(e) for e in obj]
        elements.sort()
        return '[' + ','.join(elements) + ']'
    else:
        return repr(obj)

def compare(obj1, obj2, path=''):
    differences = []
    if type(obj1) != type(obj2):
        differences.append('Type mismatch at {}: {} vs {}'.format(path, type(obj1).__name__, type(obj2).__name__))
        return differences
    if isinstance(obj1, dict):
        keys1 = set(obj1.keys())
        keys2 = set(obj2.keys())
        for key in keys1.union(keys2):
            new_path = '{}["{}"]'.format(path, key)
            if key in obj1 and key in obj2:
                differences.extend(compare(obj1[key], obj2[key], path=new_path))
            elif key in obj1:
                differences.append('Key "{}" missing in second object at {}'.format(key, new_path))
            else:
                differences.append('Key "{}" missing in first object at {}'.format(key, new_path))
    elif isinstance(obj1, list):
        counter1 = collections.Counter([canonical_serialize(e) for e in obj1])
        counter2 = collections.Counter([canonical_serialize(e) for e in obj2])
        all_elements = set(counter1.keys()).union(counter2.keys())
        for elem in all_elements:
            count1 = counter1.get(elem, 0)
            count2 = counter2.get(elem, 0)
            if count1 != count2:
                differences.append('Element {} has count {} vs {} at {}'.format(elem, count1, count2, path))
    else:
        if obj1 != obj2:
            differences.append('Value mismatch at {}: {} vs {}'.format(path, repr(obj1), repr(obj2)))
    return differences

def main():
    if len(sys.argv) != 3:
        print('Usage: {} file1.json file2.json'.format(sys.argv[0]))
        sys.exit(1)
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    with open(file1, 'r') as f1:
        obj1 = json.load(f1)
    with open(file2, 'r') as f2:
        obj2 = json.load(f2)
    differences = compare(obj1, obj2)
    if differences:
        print('Differences found:')
        for diff in differences:
            print(diff)
    else:
        print('The two JSON files contain the same content.')

if __name__ == '__main__':
    main()
