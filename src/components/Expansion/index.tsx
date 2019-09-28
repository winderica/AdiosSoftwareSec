import { Divider, ExpansionPanel, ExpansionPanelDetails, ExpansionPanelSummary, Theme, Typography } from '@material-ui/core';
import { createStyles, makeStyles } from '@material-ui/styles';
import React, { FC } from 'react';
import { CodeHighlight } from '../Highlight';

const useStyles = makeStyles(({ typography }: Theme) => createStyles({
    codeContainer: {
        fontSize: typography.h6.fontSize,
        flexDirection: 'column',
        maxHeight: '70vh',
        overflowY: 'auto'
    }
}));

export const Expansion: FC<{ code1: string, code2?: string }> = ({ code1, code2 }) => {
    const classes = useStyles();
    return <ExpansionPanel>
        <ExpansionPanelSummary>sample code</ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.codeContainer}>
            <Typography variant='h4'>code 1</Typography>
            <Divider />
            <CodeHighlight language='cpp'>{code1}</CodeHighlight>
            {code2 && <>
                <Typography variant='h4'>code 2</Typography>
                <Divider />
                <CodeHighlight language='cpp'>{code2}</CodeHighlight>
            </>}
        </ExpansionPanelDetails>
    </ExpansionPanel>;
};
