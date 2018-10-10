# Hardware

## Architecture 
__-- or |  :__ RS-485 line.
__== or || :__ Power line.
__s<ID>:__ slave with ID.
__p<ID>    :__ power module with ID.
__+:__ Led transmitter and controller.
__**:__ 220V powerline

_The Ground Is NOT Illustrated below._


```
becon
master============================================
  |    ||      ||      ||      ||      ||      ||
  |    ||      ||      ||      ||      ||      ||
  |----s1-+    s2-+    s3-+    s4-+    s5-+    s6-+
        |       |       |       |       |       |
        |       |       |       |       |       |
220V***p1******p2******p3******p4******p5******p6

```

## Setting ID
Before installation, you MUST set diffrent IDs for each device ( including
the master and the slaves ).

You can set ID by setting the DIP switch on the adapter board which is green
and also called _hole board_.

The ID 0 is for the master which calculate the location of each tag. After
that, it sends data to each slave device through RS-485.

So, the ID 1 to 6 is for the slave devices.

_ATTENTION: master DOES NOT SEND ANY DATA TO LED._


# Data Stream
## Data Structure on RS-485

```
^B<Beacon number>T<tag number>X<X value>Y<Y value>$%
```

Example:
> ^B1T0X3.679Y2.233$%
> ^B4T1X1.67333Y0.2333$%

## Data Structure on light
Same as upon.
