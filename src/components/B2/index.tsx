import { Button, MenuItem, Table, TableBody, TableCell, TableHead, TableRow, TextField } from '@material-ui/core';
import { useSnackbar } from 'notistack';
import React, { ChangeEvent, useState } from 'react';
import codes from '../../samples/B2.json';
import { useStyles } from '../../styles/style';
import { getTypes } from '../../utils/getTypes';
import { Expansion } from '../Expansion';

interface Result {
    description: string;
    left: string;
    right: string;
    lineNumber: number;
    lineOffset: number;
}

export const B2 = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [result, setResult] = useState<Result[]>([]);
    const classes = useStyles();
    const [code1, setCode1] = useState(0);
    const [types, setTypes] = useState('');

    const run = () => {
        try {
            setResult(JSON.parse(window.main.intWidthOverflow(codes[code1], getTypes(types))));
        } catch ({ message }) {
            enqueueSnackbar(message, { variant: 'error' });
        }
    };

    const setCode = (event: ChangeEvent<{ name?: string; value: unknown }>) => {
        setCode1(event.target.value as number);
        setResult([]);
    };

    const handleInput = (event: ChangeEvent<HTMLInputElement>) => {
        setTypes(event.target.value);
    };

    return (
        <div>
            <Expansion code1={codes[code1]} />
            <div className={classes.itemContainer}>
                <TextField
                    select
                    label='code1'
                    value={code1}
                    onChange={setCode}
                >
                    {codes.map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}

                </TextField>
                <TextField
                    label='types'
                    value={types}
                    onChange={handleInput}
                />
                <Button onClick={run}>Detect Int Width Overflow</Button>
            </div>
            <div className={classes.resultContainer}>
                <Table>
                    <TableHead>
                        <TableRow>
                            <TableCell>left</TableCell>
                            <TableCell>right</TableCell>
                            <TableCell>lineNumber</TableCell>
                            <TableCell>lineOffset</TableCell>
                            <TableCell>description</TableCell>
                        </TableRow>
                    </TableHead>
                    <TableBody>
                        {result.map((i, j) => (
                            <TableRow key={j}>
                                <TableCell component='th' scope='row'>{i.left}</TableCell>
                                <TableCell component='th' scope='row'>{i.right}</TableCell>
                                <TableCell>{i.lineNumber}</TableCell>
                                <TableCell>{i.lineOffset}</TableCell>
                                <TableCell>{i.description}</TableCell>
                            </TableRow>
                        ))}
                    </TableBody>
                </Table>
            </div>
        </div>
    );
};
