import { Theme } from '@material-ui/core';
import { createStyles, makeStyles } from '@material-ui/styles';

export const useStyles = makeStyles(({ typography, spacing }: Theme) => createStyles({
    codeContainer: {
        fontSize: typography.h6.fontSize,
        flexDirection: 'column',
        maxHeight: '70vh',
        overflowY: 'auto'
    },
    itemContainer: {
        display: 'flex',
        marginTop: spacing(2),
        '& > *': {
            margin: spacing(1),
        }
    },
    resultContainer: {
        marginTop: spacing(2),
        '& > *': {
            margin: spacing(1),
        }
    }
}));
