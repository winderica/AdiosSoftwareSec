import { AppBar, Tab, Tabs, Theme } from '@material-ui/core';
import { createStyles, makeStyles } from '@material-ui/styles';
import { SnackbarProvider } from 'notistack';
import React, { useState } from 'react';
import { A2 } from './components/A2';
import { B1 } from './components/B1';
import { B2 } from './components/B2';
import { B3 } from './components/B3';
import { B6 } from './components/B6';
import { R2 } from './components/R2';
import { R3 } from './components/R3';
import { R4 } from './components/R4';
import { R5 } from './components/R5';
import withRoot from './styles/withRoot';

const useStyles = makeStyles(({ spacing }: Theme) => createStyles({
    container: {
        padding: spacing(3)
    },
}));

const App = () => {
    const classes = useStyles();
    const [tab, setTab] = useState(0);

    const handleSetTab = (event: React.ChangeEvent<{}>, newTab: number) => {
        setTab(newTab);
    };

    return (
        <SnackbarProvider maxSnack={3}>
            <div className={classes.container}>
                <AppBar position='static'>
                    <Tabs value={tab} onChange={handleSetTab} variant='scrollable' scrollButtons='auto'>
                        <Tab label='compare string' />
                        <Tab label='compare cfg' />
                        <Tab label='stack overflow' />
                        <Tab label='format string' />
                        <Tab label='multithread' />
                        <Tab label='heap overflow' />
                        <Tab label='int width overflow' />
                        <Tab label='int arith overflow' />
                        <Tab label='race condition' />
                    </Tabs>
                </AppBar>
                {[<R2 />, <R3 />, <R4 />, <R5 />, <A2 />, <B1 />, <B2 />, <B3 />, <B6 />][tab]}
            </div>
        </SnackbarProvider>
    );
};

export default withRoot(App);
