package com.witted.service.model;

import java.util.List;

public class HeartBeat {


    /**
     * interval : 30
     * devs : []
     */

    private int interval;
    private List<?> devs;

    public HeartBeat(int interval, List<?> devs) {
        this.interval = interval;
        this.devs = devs;
    }

    public int getInterval() {
        return interval;
    }

    public void setInterval(int interval) {
        this.interval = interval;
    }

    public List<?> getDevs() {
        return devs;
    }

    public void setDevs(List<?> devs) {
        this.devs = devs;
    }
}
