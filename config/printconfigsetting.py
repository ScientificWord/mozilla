import configobj, sys

try:
    (file, section, key) = sys.argv[1:]
except ValueError:
    print("Usage: printconfigsetting.py <file> <section> <setting>")
    sys.exit(1)

c = configobj.ConfigObj(file)

try:
    s = c[section]
except KeyError:
    print("Section [%s] not found." % section, file=sys.stderr)
    sys.exit(1)

try:
    print(s[key])
except KeyError:
    print("Key %s not found." % key, file=sys.stderr)
    sys.exit(1)
